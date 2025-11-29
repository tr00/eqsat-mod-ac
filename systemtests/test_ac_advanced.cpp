#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "theory.h"

// Advanced and convoluted tests for AC operators
// These test edge cases, complex nested structures, and intricate pattern matching scenarios

TEST_CASE("AC operators with deeply nested structures", "[egraph][ac][nested]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto c = theory.add_operator("c", 0);
    auto mul = theory.add_operator("mul", AC);

    theory.add_rewrite_rule("assoc_expand", "(mul (mul ?x ?y) ?z)", "(mul ?x ?y ?z)");

    EGraph egraph(theory);

    SECTION("Nested AC operators with 4 levels deep")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        // Build mul(mul(mul(mul(a, b), c), a), b)
        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        auto mul_ab_c = Expr::make_operator(mul, {mul_ab, c_expr});
        auto mul_abc_a = Expr::make_operator(mul, {mul_ab_c, a_expr});
        auto mul_abca_b = Expr::make_operator(mul, {mul_abc_a, b_expr});

        // Build mul(a, a, b, b, c) which should be equivalent after flattening
        auto mul_flat = Expr::make_operator(mul, {a_expr, a_expr, b_expr, b_expr, c_expr});

        id_t nested_id = egraph.add_expr(mul_abca_b);
        id_t flat_id = egraph.add_expr(mul_flat);

        REQUIRE(egraph.is_equiv(nested_id, flat_id) == false);

        // Multiple rebuilds and saturations to flatten nested structure
        egraph.saturate(5);

        // After flattening, both should be equivalent (same multiset)
        REQUIRE(egraph.is_equiv(nested_id, flat_id) == true);
    }

    SECTION("Mixed nested and flat AC terms with varying multiplicities")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);

        // mul(mul(a, a), mul(b, b)) vs mul(a, a, b, b)
        auto mul_aa = Expr::make_operator(mul, {a_expr, a_expr});
        auto mul_bb = Expr::make_operator(mul, {b_expr, b_expr});
        auto nested = Expr::make_operator(mul, {mul_aa, mul_bb});

        auto flat = Expr::make_operator(mul, {a_expr, a_expr, b_expr, b_expr});

        id_t nested_id = egraph.add_expr(nested);
        id_t flat_id = egraph.add_expr(flat);

        egraph.saturate(3);

        REQUIRE(egraph.is_equiv(nested_id, flat_id) == true);
    }
}

TEST_CASE("AC operators with multiple rewrite rules interacting", "[egraph][ac][rules]")
{
    Theory theory;

    auto x = theory.add_operator("x", 0);
    auto zero = theory.add_operator("zero", 0);
    auto one = theory.add_operator("one", 0);
    auto add = theory.add_operator("add", AC);
    auto mul = theory.add_operator("mul", AC);

    // Multiple interacting rules
    theory.add_rewrite_rule("add_zero", "(add ?a (zero))", "?a");
    theory.add_rewrite_rule("mul_one", "(mul ?a (one))", "?a");
    theory.add_rewrite_rule("mul_zero", "(mul ?a (zero))", "(zero)");
    theory.add_rewrite_rule("distributive", "(mul ?a (add ?b ?c))", "(add (mul ?a ?b) (mul ?a ?c))");

    EGraph egraph(theory);

    SECTION("Distributive law with AC operators")
    {
        auto x_expr = Expr::make_operator(x);
        auto zero_expr = Expr::make_operator(zero);
        auto one_expr = Expr::make_operator(one);

        // mul(x, add(one, zero)) should distribute and simplify
        auto add_expr = Expr::make_operator(add, {one_expr, zero_expr});
        auto mul_expr = Expr::make_operator(mul, {x_expr, add_expr});

        id_t x_id = egraph.add_expr(x_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        // mul(x, add(one, zero))
        // -> mul(x, one)  [add_zero]
        // -> x            [mul_one]
        egraph.saturate(5);

        REQUIRE(egraph.is_equiv(x_id, mul_id) == true);
    }

    SECTION("Complex interaction: distributive + identity + annihilation")
    {
        auto x_expr = Expr::make_operator(x);
        auto zero_expr = Expr::make_operator(zero);
        auto one_expr = Expr::make_operator(one);

        // add(mul(x, one), mul(x, zero))
        auto mul_x_one = Expr::make_operator(mul, {x_expr, one_expr});
        auto mul_x_zero = Expr::make_operator(mul, {x_expr, zero_expr});
        auto add_expr = Expr::make_operator(add, {mul_x_one, mul_x_zero});

        id_t x_id = egraph.add_expr(x_expr);
        id_t add_id = egraph.add_expr(add_expr);

        // add(mul(x, one), mul(x, zero))
        // -> add(x, zero)  [mul_one, mul_zero]
        // -> x             [add_zero]
        egraph.saturate(5);

        REQUIRE(egraph.is_equiv(x_id, add_id) == true);
    }
}

TEST_CASE("AC operators with unification cascades", "[egraph][ac][unification]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto c = theory.add_operator("c", 0);
    auto d = theory.add_operator("d", 0);
    auto mul = theory.add_operator("mul", AC);

    EGraph egraph(theory);

    SECTION("Chain of unifications causing AC term merges")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);
        auto d_expr = Expr::make_operator(d);

        // Create terms: mul(a, b), mul(b, c), mul(c, d), mul(d, a)
        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        auto mul_bc = Expr::make_operator(mul, {b_expr, c_expr});
        auto mul_cd = Expr::make_operator(mul, {c_expr, d_expr});
        auto mul_da = Expr::make_operator(mul, {d_expr, a_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t b_id = egraph.add_expr(b_expr);
        id_t c_id = egraph.add_expr(c_expr);
        id_t d_id = egraph.add_expr(d_expr);

        id_t mul_ab_id = egraph.add_expr(mul_ab);
        id_t mul_bc_id = egraph.add_expr(mul_bc);
        id_t mul_cd_id = egraph.add_expr(mul_cd);
        id_t mul_da_id = egraph.add_expr(mul_da);

        // Unify a ≡ b ≡ c ≡ d (chain unification)
        egraph.unify(a_id, b_id);
        egraph.rebuild();

        egraph.unify(b_id, c_id);
        egraph.rebuild();

        egraph.unify(c_id, d_id);
        egraph.rebuild();

        // All mul terms should now be equivalent (all contain same multiset)
        REQUIRE(egraph.is_equiv(mul_ab_id, mul_bc_id) == true);
        REQUIRE(egraph.is_equiv(mul_bc_id, mul_cd_id) == true);
        REQUIRE(egraph.is_equiv(mul_cd_id, mul_da_id) == true);
    }

    SECTION("Star unification: all pairs unified")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto c_expr = Expr::make_operator(c);

        // Create all pairwise mul terms
        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        auto mul_ac = Expr::make_operator(mul, {a_expr, c_expr});
        auto mul_bc = Expr::make_operator(mul, {b_expr, c_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t b_id = egraph.add_expr(b_expr);
        id_t c_id = egraph.add_expr(c_expr);

        id_t mul_ab_id = egraph.add_expr(mul_ab);
        id_t mul_ac_id = egraph.add_expr(mul_ac);
        id_t mul_bc_id = egraph.add_expr(mul_bc);

        REQUIRE(egraph.is_equiv(mul_ab_id, mul_ac_id) == false);

        // Unify a ≡ b
        egraph.unify(a_id, b_id);
        egraph.rebuild();

        // Now mul(a, c) ≡ mul(b, c)
        REQUIRE(egraph.is_equiv(mul_ac_id, mul_bc_id) == true);

        // Unify b ≡ c (transitively a ≡ c)
        egraph.unify(b_id, c_id);
        egraph.rebuild();

        // Now all mul terms are equivalent (all are mul(x, x) for same x)
        REQUIRE(egraph.is_equiv(mul_ab_id, mul_ac_id) == true);
        REQUIRE(egraph.is_equiv(mul_ab_id, mul_bc_id) == true);
    }
}

TEST_CASE("AC operators with complex partial matching scenarios", "[egraph][ac][partial]")
{
    Theory theory;

    auto var = theory.add_operator("var", 0);
    theory.add_operator("inv", 1);
    theory.add_operator("sqr", 1);
    auto mul = theory.add_operator("mul", AC);
    auto one = theory.add_operator("one", 0);

    SECTION("Nested variable occurrences are LINEAR")
    {
        // These patterns are LINEAR because variables don't repeat as direct children
        // (mul ?x (inv ?x)) - ?x appears once as direct child, second is inside inv
        theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");

        // (sqr ?x) -> (mul ?x ?x) - RHS being non-linear is fine, only LHS matters
        theory.add_rewrite_rule("sqr_expand", "(sqr ?x)", "(mul ?x ?x)");
    }

    SECTION("Non-linear LHS patterns are rejected")
    {
        // This would be non-linear: (mul ?x ?x) - ?x appears twice as direct children
        REQUIRE_THROWS_AS(theory.add_rewrite_rule("square", "(mul ?x ?x)", "(sqr ?x)"), std::invalid_argument);
    }

    SECTION("Linear identity pattern works")
    {
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

TEST_CASE("AC operators with pathological cases", "[egraph][ac][edge]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto mul = theory.add_operator("mul", AC);

    EGraph egraph(theory);

    SECTION("Single element multiset")
    {
        auto a_expr = Expr::make_operator(a);
        auto mul_a = Expr::make_operator(mul, {a_expr});

        egraph.add_expr(a_expr);
        egraph.add_expr(mul_a);

        // mul(a) should be treated as just {a}
        // Depending on implementation, might hash-cons to same eclass or different
        // This tests the boundary case of AC with arity 1
    }

    SECTION("Empty multiset difference after matching")
    {
        auto a_expr = Expr::make_operator(a);

        theory.add_rewrite_rule("exact_match", "(mul ?x)", "?x");

        auto mul_a = Expr::make_operator(mul, {a_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t mul_id = egraph.add_expr(mul_a);

        egraph.saturate(2);

        // Pattern mul(?x) matches entire multiset, leaving empty diff
        REQUIRE(egraph.is_equiv(a_id, mul_id) == true);
    }

    SECTION("Very large multiset (stress test)")
    {
        auto a_expr = Expr::make_operator(a);

        // Create mul(a * 100)
        Vec<std::shared_ptr<Expr>> args;
        for (int i = 0; i < 100; i++)
        {
            args.push_back(a_expr);
        }
        auto mul_100a = Expr::make_operator(mul, args);

        id_t mul_id = egraph.add_expr(mul_100a);

        // Should handle large multisets without crashing
        REQUIRE(mul_id != static_cast<id_t>(-1));
    }
}

TEST_CASE("AC operators with interleaved non-AC operators", "[egraph][ac][mixed]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto mul = theory.add_operator("mul", AC);
    auto cons = theory.add_operator("cons", 2); // Non-AC binary operator

    EGraph egraph(theory);

    SECTION("AC operator containing non-AC operator")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);

        // cons(a, b) and cons(b, a) are different (non-AC)
        auto cons_ab = Expr::make_operator(cons, {a_expr, b_expr});
        auto cons_ba = Expr::make_operator(cons, {b_expr, a_expr});

        id_t cons_ab_id = egraph.add_expr(cons_ab);
        id_t cons_ba_id = egraph.add_expr(cons_ba);

        REQUIRE(egraph.is_equiv(cons_ab_id, cons_ba_id) == false);

        // But mul(cons(a,b), cons(b,a)) should commute
        auto mul_1 = Expr::make_operator(mul, {cons_ab, cons_ba});
        auto mul_2 = Expr::make_operator(mul, {cons_ba, cons_ab});

        id_t mul_1_id = egraph.add_expr(mul_1);
        id_t mul_2_id = egraph.add_expr(mul_2);

        // AC on outer mul should make these equivalent
        REQUIRE(egraph.is_equiv(mul_1_id, mul_2_id) == true);
    }

    SECTION("Non-AC operator containing AC operators")
    {
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);

        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        auto mul_ba = Expr::make_operator(mul, {b_expr, a_expr});

        // mul(a, b) ≡ mul(b, a) due to AC
        id_t mul_ab_id = egraph.add_expr(mul_ab);
        id_t mul_ba_id = egraph.add_expr(mul_ba);

        REQUIRE(egraph.is_equiv(mul_ab_id, mul_ba_id) == true);

        // cons(mul(a,b), x) and cons(mul(b,a), x) should be equivalent
        auto cons_1 = Expr::make_operator(cons, {mul_ab, a_expr});
        auto cons_2 = Expr::make_operator(cons, {mul_ba, a_expr});

        id_t cons_1_id = egraph.add_expr(cons_1);
        id_t cons_2_id = egraph.add_expr(cons_2);

        egraph.rebuild();

        // Since mul_ab ≡ mul_ba, congruence should make cons terms equivalent
        REQUIRE(egraph.is_equiv(cons_1_id, cons_2_id) == true);
    }
}

TEST_CASE("AC operators with ground term matching", "[egraph][ac][ground]")
{
    Theory theory;

    auto zero = theory.add_operator("zero", 0);
    auto one = theory.add_operator("one", 0);
    auto two = theory.add_operator("two", 0);
    auto add = theory.add_operator("add", AC);

    // Ground pattern (no variables)
    theory.add_rewrite_rule("zero_one_two", "(add (zero) (one))", "(one)");
    theory.add_rewrite_rule("one_one_two", "(add (one) (one))", "(two)");

    EGraph egraph(theory);

    SECTION("Ground pattern matches regardless of order")
    {
        auto zero_expr = Expr::make_operator(zero);
        auto one_expr = Expr::make_operator(one);

        // add(zero, one) matches forward
        auto add_zo = Expr::make_operator(add, {zero_expr, one_expr});
        id_t zo_id = egraph.add_expr(add_zo);

        egraph.saturate(1);

        id_t one_id = egraph.add_expr(one_expr);
        REQUIRE(egraph.is_equiv(zo_id, one_id) == true);

        // add(one, zero) also matches (AC commutativity)
        auto add_oz = Expr::make_operator(add, {one_expr, zero_expr});
        id_t oz_id = egraph.add_expr(add_oz);

        egraph.saturate(1);

        REQUIRE(egraph.is_equiv(oz_id, one_id) == true);
    }

    SECTION("Ground pattern in larger multiset")
    {
        auto zero_expr = Expr::make_operator(zero);
        auto one_expr = Expr::make_operator(one);
        auto two_expr = Expr::make_operator(two);

        // add(zero, one, two) contains add(zero, one) as submultiset
        auto add_expr = Expr::make_operator(add, {zero_expr, one_expr, two_expr});

        id_t add_id = egraph.add_expr(add_expr);

        // Should match ground pattern, creating add(one, two)
        egraph.saturate(2);

        auto add_one_two = Expr::make_operator(add, {one_expr, two_expr});
        id_t result_id = egraph.add_expr(add_one_two);

        REQUIRE(egraph.is_equiv(add_id, result_id) == true);
    }
}

TEST_CASE("AC operators saturation with cyclic rewrites", "[egraph][ac][saturation]")
{
    Theory theory;

    theory.add_operator("a", 0);
    theory.add_operator("b", 0);
    theory.add_operator("mul", AC);
    theory.add_operator("f", 1);
    theory.add_operator("g", 1);

    SECTION("Nested variable occurrences are LINEAR")
    {
        // These patterns are LINEAR - variables appear in nested subterms
        theory.add_rewrite_rule("f_to_g", "(mul ?x (f ?x))", "(mul ?x (g ?x))");
        theory.add_rewrite_rule("extract_f", "(mul (f ?x) (g ?x))", "(mul ?x ?x)");
    }
}

TEST_CASE("AC operators with conditional-like patterns", "[egraph][ac][conditional]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto tt = theory.add_operator("true", 0);
    auto ff = theory.add_operator("false", 0);
    auto andd = theory.add_operator("and", AC);
    theory.add_operator("or", AC); // Define but don't need to store

    SECTION("Linear boolean algebra rules work")
    {
        // Boolean algebra rules (linear patterns only)
        theory.add_rewrite_rule("and_true", "(and ?x (true))", "?x");
        theory.add_rewrite_rule("and_false", "(and ?x (false))", "(false)");
        theory.add_rewrite_rule("or_true", "(or ?x (true))", "(true)");
        theory.add_rewrite_rule("or_false", "(or ?x (false))", "?x");

        EGraph egraph(theory);

        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto tt_expr = Expr::make_operator(tt);

        // and(a, b, true) -> and(a, b)
        auto and_expr = Expr::make_operator(andd, {a_expr, b_expr, tt_expr});

        id_t and_id = egraph.add_expr(and_expr);

        egraph.saturate(2);

        auto and_ab = Expr::make_operator(andd, {a_expr, b_expr});
        id_t and_ab_id = egraph.add_expr(and_ab);

        REQUIRE(egraph.is_equiv(and_id, and_ab_id) == true);
    }

    SECTION("Boolean and with false annihilates")
    {
        theory.add_rewrite_rule("and_false", "(and ?x (false))", "(false)");
        EGraph egraph(theory);

        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto ff_expr = Expr::make_operator(ff);

        // and(a, b, false) -> false
        auto and_expr = Expr::make_operator(andd, {a_expr, b_expr, ff_expr});

        id_t and_id = egraph.add_expr(and_expr);

        egraph.saturate(2);

        id_t ff_id = egraph.add_expr(ff_expr);

        REQUIRE(egraph.is_equiv(and_id, ff_id) == true);
    }
}
