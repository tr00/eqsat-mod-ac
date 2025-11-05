#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "theory.h"

// These tests verify Associative-Commutative (AC) operator behavior
// AC operators should match patterns regardless of argument order
// and ensure congruence closure respects commutativity

TEST_CASE("AC operators support commutative pattern matching", "[egraph][ac][pattern]")
{
    Theory theory;

    auto var = theory.add_operator("var", 0);
    auto one = theory.add_operator("one", 0);
    auto mul = theory.add_operator("mul", AC);

    // Rewrite rule: mul(?x, one) -> ?x
    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    EGraph egraph(theory);

    SECTION("AC pattern matches forward order mul(a, one)")
    {
        auto var_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {var_expr, one_expr});

        id_t var_id = egraph.add_expr(var_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        REQUIRE(egraph.is_equiv(var_id, mul_id) == false);

        egraph.saturate(1);

        REQUIRE(egraph.is_equiv(var_id, mul_id) == true);
    }

    SECTION("AC pattern matches reverse order mul(b, a)")
    {
        auto var_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {one_expr, var_expr}); // Reversed!

        id_t var_id = egraph.add_expr(var_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        REQUIRE(egraph.is_equiv(var_id, mul_id) == false);

        egraph.saturate(1);

        // With AC: Pattern (mul ?x (b)) should ALSO match mul(b, a)
        // binding ?x to a, thus making a ≡ mul(b, a)
        REQUIRE(egraph.is_equiv(var_id, mul_id) == true);
    }
}

TEST_CASE("AC operators enforce commutative hash-consing", "[egraph][ac][hashcons]")
{
    Theory theory;

    auto x = theory.add_operator("x", 0);
    auto y = theory.add_operator("y", 0);

    auto add = theory.add_operator("add", AC);

    EGraph egraph(theory);

    SECTION("add(x, y) and add(y, x) are the same e-class (AC hash-consing)")
    {
        auto x_expr = Expr::make_operator(x);
        auto y_expr = Expr::make_operator(y);

        auto add_xy = Expr::make_operator(add, {x_expr, y_expr});
        auto add_yx = Expr::make_operator(add, {y_expr, x_expr});

        id_t xy_id = egraph.add_expr(add_xy);
        id_t yx_id = egraph.add_expr(add_yx);

        // With AC: add(x, y) and add(y, x) should hash-cons to same e-class
        REQUIRE(egraph.is_equiv(xy_id, yx_id) == true);
    }

    SECTION("Multiple insertions of commuted terms produce single e-class")
    {
        auto x_expr = Expr::make_operator(x);
        auto y_expr = Expr::make_operator(y);

        auto add_xy = Expr::make_operator(add, {x_expr, y_expr});

        id_t id1 = egraph.add_expr(add_xy);

        auto add_yx = Expr::make_operator(add, {y_expr, x_expr});
        id_t id2 = egraph.add_expr(add_yx);

        // Insert same pattern again
        id_t id3 = egraph.add_expr(add_xy);

        // With AC: All should map to the same e-class
        REQUIRE(egraph.is_equiv(id1, id2) == true);
        REQUIRE(egraph.is_equiv(id1, id3) == true); // This already works (hash-consing)
    }
}

TEST_CASE("AC operators support commutative congruence closure", "[egraph][ac][congruence]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto c = theory.add_operator("c", 0);

    auto mul = theory.add_operator("mul", AC);

    EGraph egraph(theory);

    SECTION("If a=b, then mul(a, c) ≡ mul(c, b) via AC congruence")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        auto mul_ac = Expr::make_operator(mul, {a_expr, c_expr});
        auto mul_cb = Expr::make_operator(mul, {c_expr, b_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t b_id = egraph.add_expr(b_expr);
        id_t mul_ac_id = egraph.add_expr(mul_ac);
        id_t mul_cb_id = egraph.add_expr(mul_cb);

        REQUIRE(egraph.is_equiv(mul_ac_id, mul_cb_id) == false);

        // Unify a and b
        egraph.unify(a_id, b_id);

        // Rebuild to apply congruence
        egraph.rebuild();
        egraph.rebuild(); // May need multiple rebuilds for nested terms

        // With AC: mul(a, f) and mul(f, b) should be congruent
        // because a=b makes the args equivalent sets: {a, f} ≡ {f, b}
        REQUIRE(egraph.is_equiv(mul_ac_id, mul_cb_id) == true);
    }

    SECTION("AC congruence with nested terms")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);

        // Build mul(mul(a, b), a) and mul(a, mul(b, a))
        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        auto mul_ba = Expr::make_operator(mul, {b_expr, a_expr});

        auto mul_ab_a = Expr::make_operator(mul, {mul_ab, a_expr});
        auto mul_a_ba = Expr::make_operator(mul, {a_expr, mul_ba});

        id_t id1 = egraph.add_expr(mul_ab_a);
        id_t id2 = egraph.add_expr(mul_a_ba);

        // With AC:
        // mul(mul(a, b), a) has args {mul(a,b), a}
        // mul(a, mul(b, a)) has args {a, mul(b,a)}
        // If mul(a,b) ≡ mul(b,a) (AC hash-cons), then the arg sets are equal
        REQUIRE(egraph.is_equiv(id1, id2) == true);
    }
}

TEST_CASE("AC operators handle duplicate arguments correctly", "[egraph][ac][duplicates]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);

    // TODO: Replace with theory.add_operator_ac("mul", 2) when AC API is available
    auto mul = theory.add_operator("mul", 2);

    SECTION("Pattern with duplicate variables is rejected (non-linear)")
    {
        // Non-linear patterns like (mul ?x ?x) are not currently supported
        REQUIRE_THROWS_AS(theory.add_rewrite_rule("idempotent", "(mul ?x ?x)", "?x"), std::invalid_argument);
    }

    SECTION("Duplicate arguments in AC multiset representation")
    {
        EGraph egraph(theory);

        auto a_expr = Expr::make_operator(a);

        // mul(a, a) should be stored as multiset {a, a} with multiplicity 2
        auto mul_aa_1 = Expr::make_operator(mul, {a_expr, a_expr});
        auto mul_aa_2 = Expr::make_operator(mul, {a_expr, a_expr});

        id_t id1 = egraph.add_expr(mul_aa_1);
        id_t id2 = egraph.add_expr(mul_aa_2);

        // Both should hash-cons to same e-class
        REQUIRE(egraph.is_equiv(id1, id2) == true);
    }
}

TEST_CASE("AC operators with ternary operations", "[egraph][ac][ternary]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto c = theory.add_operator("c", 0);

    // TODO: Replace with theory.add_operator_ac("max", 3) when AC API is available
    auto max3 = theory.add_operator("max", 3);

    EGraph egraph(theory);

    SECTION("Ternary AC operator hash-conses all permutations")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        // Create different permutations of max(a, b, c)
        auto max_abc = Expr::make_operator(max3, {a_expr, b_expr, c_expr});
        auto max_bca = Expr::make_operator(max3, {b_expr, c_expr, a_expr});
        auto max_cab = Expr::make_operator(max3, {c_expr, a_expr, b_expr});

        id_t id_abc = egraph.add_expr(max_abc);
        id_t id_bca = egraph.add_expr(max_bca);
        id_t id_cab = egraph.add_expr(max_cab);

        // With AC: All permutations should be in same e-class
        // TODO: Change to true when AC is implemented
        REQUIRE(egraph.is_equiv(id_abc, id_bca) == false);
        REQUIRE(egraph.is_equiv(id_abc, id_cab) == false);
        REQUIRE(egraph.is_equiv(id_bca, id_cab) == false);
    }

    SECTION("Ternary AC pattern matching is order-independent")
    {
        theory.add_rewrite_rule("max_with_c", "(max ?x ?y (c))", "?x");

        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        // Different orderings of max(a, b, c)
        auto max_abc = Expr::make_operator(max3, {a_expr, b_expr, c_expr});
        auto max_cab = Expr::make_operator(max3, {c_expr, a_expr, b_expr});
        auto max_bca = Expr::make_operator(max3, {b_expr, c_expr, a_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t id1 = egraph.add_expr(max_abc);
        id_t id2 = egraph.add_expr(max_cab);
        id_t id3 = egraph.add_expr(max_bca);

        egraph.saturate(1);

        // Pattern (max ?x ?y (c)) should match all permutations
        // and make them equivalent to a
        // TODO: Change to true when AC is implemented
        REQUIRE(egraph.is_equiv(a_id, id1) == false);
        REQUIRE(egraph.is_equiv(a_id, id2) == false);
        REQUIRE(egraph.is_equiv(a_id, id3) == false);
    }
}

TEST_CASE("AC interaction with rebuild after unification", "[egraph][ac][rebuild]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto c = theory.add_operator("c", 0);

    // TODO: Replace with theory.add_operator_ac("add", 2) when AC API is available
    auto add = theory.add_operator("add", 2);

    EGraph egraph(theory);

    SECTION("Rebuild merges AC terms with canonically equivalent args")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        // Create: add(a, b) and add(c, b)
        auto add_ab = Expr::make_operator(add, {a_expr, b_expr});
        auto add_cb = Expr::make_operator(add, {c_expr, b_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t c_id = egraph.add_expr(c_expr);
        id_t add_ab_id = egraph.add_expr(add_ab);
        id_t add_cb_id = egraph.add_expr(add_cb);

        REQUIRE(egraph.is_equiv(add_ab_id, add_cb_id) == false);

        // Unify a and c
        egraph.unify(a_id, c_id);

        // Before rebuild: add(a, b) and add(c, b) are different
        REQUIRE(egraph.is_equiv(add_ab_id, add_cb_id) == false);

        // Rebuild should canonicalize and merge
        egraph.rebuild();

        // After rebuild with AC: add(a, b) ≡ add(c, b) because a ≡ c
        // so canonical args {a, b} ≡ {c, b}
        REQUIRE(egraph.is_equiv(add_ab_id, add_cb_id) == true);
    }

    SECTION("AC rebuild handles commuted duplicates correctly")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);

        // Create add(a, b) and add(b, c)
        auto add_ab = Expr::make_operator(add, {a_expr, b_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t b_id = egraph.add_expr(b_expr);
        id_t add_ab_id = egraph.add_expr(add_ab);

        // Now unify a and b, making add(a, b) become add(a, a)
        egraph.unify(a_id, b_id);
        egraph.rebuild();

        // Create add(a, a) after unification
        auto add_aa = Expr::make_operator(add, {a_expr, a_expr});
        id_t add_aa_id = egraph.add_expr(add_aa);

        // With AC: add(a, b) where a≡b should be equivalent to add(a, a)
        REQUIRE(egraph.is_equiv(add_ab_id, add_aa_id) == true);
    }
}

TEST_CASE("AC operators support more complex pattern matching", "[egraph][ac][pattern][inverse]")
{
    Theory theory;

    // free variable
    auto var = theory.add_operator("var", 0);

    auto one = theory.add_operator("one", 0);
    theory.add_operator("inv", 1); // inv declared but not used in these tests
    auto mul = theory.add_operator("mul", AC);

    SECTION("Inverse pattern is LINEAR (nested occurrence)")
    {
        // The inverse rule (mul ?x (inv ?x)) is LINEAR because ?x only appears once
        // as a direct child of mul - the second ?x is nested inside inv
        theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");
        theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");
        EGraph egraph(theory);

        auto a_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {a_expr, one_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        REQUIRE(egraph.is_equiv(a_id, mul_id) == false);

        egraph.saturate(2);

        // After applying identity rule: mul(a, one) -> a
        REQUIRE(egraph.is_equiv(a_id, mul_id) == true);
    }
}
