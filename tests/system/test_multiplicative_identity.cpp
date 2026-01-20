#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "theory.h"

using namespace eqsat;

TEST_CASE("EGraph applies multiplicative identity rewrite rule", "[egraph][rewrite][identity]")
{
    // Create theory with operators: one, var, mul
    Theory theory;

    auto one = theory.add_operator("one", 0);
    auto var = theory.add_operator("var", 0);
    auto mul = theory.add_operator("mul", 2);

    // Add rewrite rule: mul ?x (one) -> ?x (multiplicative identity)
    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    // Create e-graph with the theory
    EGraph egraph(theory);

    SECTION("Multiplicative identity is applied during saturation")
    {
        // Build expressions:
        //   var_expr = var
        //   one_expr = one
        //   mul_expr = mul(var, one)
        auto var_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {var_expr, one_expr});

        // Insert expressions into e-graph
        id_t var_id = egraph.add_expr(var_expr);
        id_t one_id = egraph.add_expr(one_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        // Verify all expressions have different IDs initially
        REQUIRE(var_id != one_id);
        REQUIRE(var_id != mul_id);
        REQUIRE(one_id != mul_id);

        // Before saturation: var and mul(var, one) should NOT be equivalent
        bool equiv_before = egraph.is_equiv(var_id, mul_id);
        REQUIRE(equiv_before == false);

        // Run saturation with depth = 1
        egraph.saturate(1);

        // After saturation: var and mul(var, one) SHOULD be equivalent
        // because the rewrite rule (mul ?x (one)) -> ?x should have been applied
        bool equiv_after = egraph.is_equiv(var_id, mul_id);
        REQUIRE(equiv_after == true);
    }

    SECTION("Multiplicative identity with different argument order")
    {
        // Build expressions:
        //   var_expr = var
        //   one_expr = one
        //   mul_expr = mul(one, var)  // Note: reversed order
        auto var_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);
        auto mul_expr = Expr::make_operator(mul, {one_expr, var_expr});

        // Insert expressions into e-graph
        id_t var_id = egraph.add_expr(var_expr);
        id_t mul_id = egraph.add_expr(mul_expr);

        // Before saturation: should not be equivalent
        REQUIRE(egraph.is_equiv(var_id, mul_id) == false);

        // Run saturation
        egraph.saturate(1);

        // After saturation: should be equivalent
        // (the rewrite rule should match regardless of argument order if mul is AC)
        REQUIRE(egraph.is_equiv(var_id, mul_id) == false);
    }

    SECTION("Multiplicative identity does not affect other terms")
    {
        // Build expressions:
        //   var_expr = var
        //   one_expr = one
        auto var_expr = Expr::make_operator(var);
        auto one_expr = Expr::make_operator(one);

        // Insert expressions
        id_t var_id = egraph.add_expr(var_expr);
        id_t one_id = egraph.add_expr(one_expr);

        // Sanity check: var and one should not be equivalent before saturation
        REQUIRE(egraph.is_equiv(var_id, one_id) == false);

        // Run saturation
        egraph.saturate(1);

        // After saturation: var and one should STILL not be equivalent
        // The rewrite rule should not cause unrelated terms to become equivalent
        REQUIRE(egraph.is_equiv(var_id, one_id) == false);
    }
}
