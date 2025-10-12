#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "symbol_table.h"
#include "theory.h"

TEST_CASE("EGraph can do simple pattern matching", "[egraph]")
{
    Theory theory;

    auto one = theory.add_operator("one", 0);
    auto var = theory.add_operator("var", 0);
    auto mul = theory.add_operator("mul", AC);

    EGraph egraph(theory);

    // 1 * v
    auto var_expr = Expr::make_operator(var);
    auto one_expr = Expr::make_operator(one);
    auto mul_expr = Expr::make_operator(mul, {one_expr, var_expr});

    auto var_id = egraph.add_expr(var_expr);
    auto mul_id = egraph.add_expr(mul_expr);

    REQUIRE(egraph.is_equiv(var_id, mul_id) == false);

    egraph.saturate(1);
}

TEST_CASE("EGraph can insert simple terms", "[egraph][basic]")
{
    // Create theory
    Theory theory;

    // Create some operators
    Symbol zero_sym = theory.intern("0");
    Symbol one_sym = theory.intern("1");
    Symbol add_sym = theory.intern("+");
    Symbol mul_sym = theory.intern("*");

    // Add operators to theory
    theory.add_operator(zero_sym, 0);
    theory.add_operator(one_sym, 0);
    theory.add_operator(add_sym, 2);
    theory.add_operator(mul_sym, 2);

    // Create EGraph with theory
    EGraph egraph(theory);

    SECTION("Insert constant terms")
    {
        // Create constant expressions
        auto zero_expr = Expr::make_operator(zero_sym);
        auto one_expr = Expr::make_operator(one_sym);

        // Insert terms into e-graph
        id_t zero_id = egraph.add_expr(zero_expr);
        id_t one_id = egraph.add_expr(one_expr);

        // Should get different IDs for different terms
        REQUIRE(zero_id != one_id);

        // Inserting the same term again should return the same ID
        id_t zero_id2 = egraph.add_expr(zero_expr);
        REQUIRE(zero_id == zero_id2);
    }

    SECTION("Insert composite terms")
    {
        // Create expressions: 0, 1, (+ 0 1)
        auto zero_expr = Expr::make_operator(zero_sym);
        auto one_expr = Expr::make_operator(one_sym);
        auto add_expr = Expr::make_operator(add_sym, {zero_expr, one_expr});

        // Insert terms
        id_t zero_id = egraph.add_expr(zero_expr);
        id_t one_id = egraph.add_expr(one_expr);
        id_t add_id = egraph.add_expr(add_expr);

        // All should have different IDs
        REQUIRE(zero_id != one_id);
        REQUIRE(zero_id != add_id);
        REQUIRE(one_id != add_id);

        // Inserting equivalent expression should return same ID
        auto add_expr2 = Expr::make_operator(add_sym, {zero_expr, one_expr});
        id_t add_id2 = egraph.add_expr(add_expr2);
        REQUIRE(add_id == add_id2);
    }

    SECTION("Insert nested terms")
    {
        // Create expressions: 0, 1, (+ 0 1), (* (+ 0 1) 1)
        auto zero_expr = Expr::make_operator(zero_sym);
        auto one_expr = Expr::make_operator(one_sym);
        auto add_expr = Expr::make_operator(add_sym, {zero_expr, one_expr});
        auto mul_expr = Expr::make_operator(mul_sym, {add_expr, one_expr});

        // Insert nested term
        id_t mul_id = egraph.add_expr(mul_expr);

        // Should succeed and return a valid ID
        REQUIRE(mul_id > 0);

        // Inserting again should return same ID
        id_t mul_id2 = egraph.add_expr(mul_expr);
        REQUIRE(mul_id == mul_id2);
    }

    SECTION("Cannot insert pattern variables")
    {
        // Create a pattern variable
        Symbol x_sym = theory.intern("x");
        auto var_expr = Expr::make_variable(x_sym);

        // Should throw when trying to insert a variable
        REQUIRE_THROWS_AS(egraph.add_expr(var_expr), std::runtime_error);
    }
}
