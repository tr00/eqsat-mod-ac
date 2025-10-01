#include "../src/compiler.h"
#include "../src/symbol_table.h"
#include "../src/theory.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("Simple expression compilation", "[pattern_compiler]")
{
    // Create a simple expression: f()
    Theory theory;
    Symbol f = theory.intern("f");

    auto expr = Expr::make_operator(f);

    Compiler compiler;
    Symbol rule_name = theory.intern("test_rule");
    RewriteRule rule(rule_name, expr, expr);
    auto [query, subst] = compiler.compile(rule);

    // Should have one constraint: f(0)
    REQUIRE(query.constraints.size() == 1);
    REQUIRE(query.constraints[0].operator_symbol == f);
    REQUIRE(query.constraints[0].variables.size() == 1);
    REQUIRE(query.constraints[0].variables[0] == 0);

    // Head should contain the root variable
    REQUIRE(query.head.size() == 1);
    REQUIRE(query.head[0] == 0);
}

TEST_CASE("Nested expression compilation", "[pattern_compiler]")
{
    // Create expression: g(f(), h())
    Theory theory;
    Symbol f = theory.intern("f");
    Symbol g = theory.intern("g");
    Symbol h = theory.intern("h");

    auto f_expr = Expr::make_operator(f);
    auto h_expr = Expr::make_operator(h);
    auto g_expr = Expr::make_operator(g, Vec<std::shared_ptr<Expr>>{f_expr, h_expr});

    Compiler compiler;
    Symbol rule_name = theory.intern("test_rule");
    RewriteRule rule(rule_name, g_expr, g_expr);
    auto [query, subst] = compiler.compile(rule);

    // Should have three constraints: f(1), h(2), g(0, 1, 2)
    REQUIRE(query.constraints.size() == 3);

    // First constraint: f(0)
    REQUIRE(query.constraints[0].operator_symbol == f);
    REQUIRE(query.constraints[0].variables.size() == 1);
    REQUIRE(query.constraints[0].variables[0] == 1);

    // Second constraint: h(1)
    REQUIRE(query.constraints[1].operator_symbol == h);
    REQUIRE(query.constraints[1].variables.size() == 1);
    REQUIRE(query.constraints[1].variables[0] == 2);

    // Third constraint: g(2, 0, 1)
    REQUIRE(query.constraints[2].operator_symbol == g);
    REQUIRE(query.constraints[2].variables.size() == 3);
    REQUIRE(query.constraints[2].variables[0] == 0);
    REQUIRE(query.constraints[2].variables[1] == 1);
    REQUIRE(query.constraints[2].variables[2] == 2);

    // Head should contain the root variable
    REQUIRE(query.head.size() == 1);
    REQUIRE(query.head[0] == 0);
}

TEST_CASE("Deeply nested expression compilation", "[pattern_compiler]")
{
    // Create expression: add(mul(x, y), z) where x, y, z are variables
    Theory theory;
    Symbol add = theory.intern("add");
    Symbol mul = theory.intern("mul");
    Symbol x = theory.intern("x");
    Symbol y = theory.intern("y");
    Symbol z = theory.intern("z");

    auto x_expr = Expr::make_variable(x);
    auto y_expr = Expr::make_variable(y);
    auto z_expr = Expr::make_variable(z);
    auto mul_expr = Expr::make_operator(mul, Vec<std::shared_ptr<Expr>>{x_expr, y_expr});
    auto add_expr = Expr::make_operator(add, Vec<std::shared_ptr<Expr>>{mul_expr, z_expr});

    Compiler compiler;
    Symbol rule_name = theory.intern("test_rule");
    RewriteRule rule(rule_name, add_expr, add_expr);
    auto [query, subst] = compiler.compile(rule);

    // mul(1, 2, 3), add(0, 1, 4)
    REQUIRE(query.constraints.size() == 2);

    // mul(1, 2, 3)
    REQUIRE(query.constraints[0].operator_symbol == mul);
    REQUIRE(query.constraints[0].variables.size() == 3);
    REQUIRE(query.constraints[0].variables[0] == 1);
    REQUIRE(query.constraints[0].variables[1] == 2);
    REQUIRE(query.constraints[0].variables[2] == 3);

    // add(0, 1, 4)
    REQUIRE(query.constraints[1].operator_symbol == add);
    REQUIRE(query.constraints[1].variables.size() == 3);
    REQUIRE(query.constraints[1].variables[0] == 0);
    REQUIRE(query.constraints[1].variables[1] == 1);
    REQUIRE(query.constraints[1].variables[2] == 4);

    REQUIRE(query.head.size() == 4);
    REQUIRE(query.head[0] == 2);
    REQUIRE(query.head[1] == 3);
    REQUIRE(query.head[2] == 4);
    REQUIRE(query.head[3] == 0);
}

TEST_CASE("Multiple patterns compilation", "[pattern_compiler]")
{
    Theory theory;
    Symbol f = theory.intern("f");
    Symbol g = theory.intern("g");

    auto f_expr = Expr::make_operator(f);
    auto g_expr = Expr::make_operator(g);

    Symbol rule1_name = theory.intern("rule1");
    Symbol rule2_name = theory.intern("rule2");
    Vec<RewriteRule> patterns = {RewriteRule(rule1_name, f_expr, f_expr), RewriteRule(rule2_name, g_expr, g_expr)};

    Compiler compiler;
    auto kernels = compiler.compile_many(patterns);

    REQUIRE(kernels.size() == 2);

    // First query: f(0)
    REQUIRE(kernels[0].first.constraints.size() == 1);
    REQUIRE(kernels[0].first.constraints[0].operator_symbol == f);
    REQUIRE(kernels[0].first.head.size() == 1);
    REQUIRE(kernels[0].first.head[0] == 0);

    // Second query: g(0) - note that variable IDs reset for each pattern
    REQUIRE(kernels[1].first.constraints.size() == 1);
    REQUIRE(kernels[1].first.constraints[0].operator_symbol == g);
    REQUIRE(kernels[1].first.head.size() == 1);
    REQUIRE(kernels[1].first.head[0] == 0);
}
