#include "../src/pattern_compiler.h"
#include "../src/symbol_table.h"
#include "../src/theory.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("Simple expression compilation", "[pattern_compiler]")
{
    // Create a simple expression: f()
    SymbolTable symbols;
    symbol_t f = symbols.intern("f");

    auto expr = Expression::make_operator(f);

    PatternCompiler compiler;
    Query query = compiler.compile_pattern(expr);

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
    SymbolTable symbols;
    symbol_t f = symbols.intern("f");
    symbol_t g = symbols.intern("g");
    symbol_t h = symbols.intern("h");

    auto f_expr = Expression::make_operator(f);
    auto h_expr = Expression::make_operator(h);
    auto g_expr = Expression::make_operator(g, std::vector<std::shared_ptr<Expression>>{f_expr, h_expr});

    PatternCompiler compiler;
    Query query = compiler.compile_pattern(g_expr);

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
    SymbolTable symbols;
    symbol_t add = symbols.intern("add");
    symbol_t mul = symbols.intern("mul");
    symbol_t x = symbols.intern("x");
    symbol_t y = symbols.intern("y");
    symbol_t z = symbols.intern("z");

    auto x_expr = Expression::make_variable(x);
    auto y_expr = Expression::make_variable(y);
    auto z_expr = Expression::make_variable(z);
    auto mul_expr = Expression::make_operator(mul, std::vector<std::shared_ptr<Expression>>{x_expr, y_expr});
    auto add_expr = Expression::make_operator(add, std::vector<std::shared_ptr<Expression>>{mul_expr, z_expr});

    PatternCompiler compiler;
    Query query = compiler.compile_pattern(add_expr);

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
    SymbolTable symbols;
    symbol_t f = symbols.intern("f");
    symbol_t g = symbols.intern("g");

    auto f_expr = Expression::make_operator(f);
    auto g_expr = Expression::make_operator(g);

    std::vector<std::shared_ptr<Expression>> patterns = {f_expr, g_expr};

    PatternCompiler compiler;
    std::vector<Query> queries = compiler.compile_patterns(patterns);

    REQUIRE(queries.size() == 2);

    // First query: f(0)
    REQUIRE(queries[0].constraints.size() == 1);
    REQUIRE(queries[0].constraints[0].operator_symbol == f);
    REQUIRE(queries[0].head.size() == 1);
    REQUIRE(queries[0].head[0] == 0);

    // Second query: g(0) - note that variable IDs reset for each pattern
    REQUIRE(queries[1].constraints.size() == 1);
    REQUIRE(queries[1].constraints[0].operator_symbol == g);
    REQUIRE(queries[1].head.size() == 1);
    REQUIRE(queries[1].head[0] == 0);
}
