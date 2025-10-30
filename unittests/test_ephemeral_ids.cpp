#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "theory.h"

TEST_CASE("Ephemeral IDs for AC partial multiset matching", "[engine][ac][ephemeral]")
{
    Theory theory;

    auto var = theory.add_operator("var", 0);
    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", AC);
    auto one = theory.add_operator("one", 0);

    // Rewrite rules:
    // mul(?x, inv(?x)) -> one
    // mul(?x, one) -> ?x
    theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");
    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    EGraph egraph(theory);

    SECTION("Pattern mul(?x, inv(?x)) matches partial multiset from mul(var, var, inv(var))")
    {
        // This is the critical test case for ephemeral IDs!
        // We have: mul(var, var, inv(var))
        // Pattern: mul(?x, inv(?x)) should match with ?x = var
        // This creates an implicit subterm mul(var, inv(var)) that doesn't exist in memo
        // Engine should create ephemeral ID for this, and apply_match should materialize it

        auto var_expr = Expr::make_operator(var);
        auto inv_var_expr = Expr::make_operator(inv, {var_expr});

        // Create mul(var, var, inv(var))
        auto mul_expr = Expr::make_operator(mul, {var_expr, var_expr, inv_var_expr});

        id_t var_id = egraph.add_expr(var_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        REQUIRE(egraph.is_equiv(var_id, mul_id) == false);

        // Saturate: should apply inverse rule creating mul(var, one)
        // Then apply identity rule making var ≡ mul(var, var, inv(var))
        egraph.saturate(3);

        // After saturation, var should be equivalent to the original mul term
        REQUIRE(egraph.is_equiv(var_id, mul_id) == true);
    }

    SECTION("Multiple ephemeral terms in same iteration")
    {
        // Test: mul(a, a, inv(a), inv(a))
        // Pattern mul(?x, inv(?x)) should match twice
        // Creating ephemeral subterms: mul(a, inv(a)) twice

        auto a_expr = Expr::make_operator(var);
        auto inv_a_expr = Expr::make_operator(inv, {a_expr});

        // mul(a, a, inv(a), inv(a))
        auto mul_expr = Expr::make_operator(mul, {a_expr, a_expr, inv_a_expr, inv_a_expr});

        egraph.add_expr(a_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        egraph.saturate(5);

        // After applying inverse rule twice and identity rules,
        // should reduce to one
        auto one_expr = Expr::make_operator(one);
        id_t one_id = egraph.add_expr(one_expr);

        REQUIRE(egraph.is_equiv(mul_id, one_id) == true);
    }

    SECTION("Ephemeral term with nested AC operators")
    {
        // Create mul(a, mul(b, inv(b)))
        // Inner pattern mul(b, inv(b)) should match and create one
        // Then outer becomes mul(a, one) which should match identity

        auto a_expr = Expr::make_operator(var);
        auto b_expr = Expr::make_operator(var);
        auto inv_b_expr = Expr::make_operator(inv, {b_expr});
        auto inner_mul = Expr::make_operator(mul, {b_expr, inv_b_expr});
        auto outer_mul = Expr::make_operator(mul, {a_expr, inner_mul});

        id_t a_id = egraph.add_expr(a_expr);
        id_t outer_id = egraph.add_expr(outer_mul);

        REQUIRE(egraph.is_equiv(a_id, outer_id) == false);

        egraph.saturate(3);

        // After reducing inner mul to one, outer should become mul(a, one) -> a
        REQUIRE(egraph.is_equiv(a_id, outer_id) == true);
    }
}

TEST_CASE("Ephemeral IDs do not interfere with normal matching", "[engine][ac][ephemeral]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto mul = theory.add_operator("mul", AC);
    auto one = theory.add_operator("one", 0);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    EGraph egraph(theory);

    SECTION("Normal FD optimization still works")
    {
        // mul(a, one) exists in memo, should use normal FD path
        auto a_expr = Expr::make_operator(a);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {a_expr, one_expr});

        id_t a_id = egraph.add_expr(a_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        REQUIRE(egraph.is_equiv(a_id, mul_id) == false);

        egraph.saturate(1);

        // Should match via normal FD lookup (no ephemeral IDs needed)
        REQUIRE(egraph.is_equiv(a_id, mul_id) == true);
    }

    SECTION("Mix of ephemeral and non-ephemeral in same query")
    {
        // Create both mul(a, b) (exists) and pattern that creates ephemeral
        auto a_expr = Expr::make_operator(a);
        auto b_expr = Expr::make_operator(b);
        auto one_expr = Expr::make_operator(one);

        // mul(a, b) exists
        auto mul_ab = Expr::make_operator(mul, {a_expr, b_expr});
        id_t mul_ab_id = egraph.add_expr(mul_ab);

        // mul(a, b, one) - pattern mul(?x, one) should match with ephemeral ?x=mul(a,b)
        auto mul_ab_one = Expr::make_operator(mul, {a_expr, b_expr, one_expr});
        id_t mul_ab_one_id = egraph.add_expr(mul_ab_one);

        egraph.saturate(2);

        // mul(a, b, one) should reduce to mul(a, b)
        REQUIRE(egraph.is_equiv(mul_ab_id, mul_ab_one_id) == true);
    }
}
