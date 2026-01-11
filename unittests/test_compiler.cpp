#include "../src/compiler.h"
#include "../src/symbol_table.h"
#include "../src/theory.h"
#include <catch2/catch_test_macros.hpp>

using namespace eqsat;

TEST_CASE("Simple expression compilation", "[pattern_compiler]")
{
    // Create a simple expression: f()
    Theory theory;
    auto f = theory.add_operator("f", 0);

    Compiler compiler(theory);
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(f)", "(f)");
    auto [query, subst] = compiler.compile(rule);

    // Should have one constraint: f(; 0) where 0 is the e-class ID (LAST position)
    REQUIRE(query.constraints.size() == 1);
    REQUIRE(query.constraints[0].symbol == f);
    REQUIRE(query.constraints[0].variables.size() == 1);
    REQUIRE(query.constraints[0].variables[0] == 0); // ID is last (and only) element

    // Head should contain the root variable
    REQUIRE(query.head.size() == 1);
    REQUIRE(query.head[0] == 0);
}

TEST_CASE("Nested expression compilation", "[pattern_compiler]")
{
    // Create expression: g(f(), h())
    Theory theory;
    auto f = theory.add_operator("f", 0);
    auto g = theory.add_operator("g", 2);
    auto h = theory.add_operator("h", 0);

    Compiler compiler(theory);
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(g (f) (h))", "(g (f) (h))");
    auto [query, subst] = compiler.compile(rule);

    // Variable assignment order (POST-ORDER): f=0, h=1, g=2 (children before parents)
    // Constraints with IDs in LAST position: f(; 0), h(; 1), g(0, 1; 2)
    REQUIRE(query.constraints.size() == 3);

    // First constraint: f(; 0) - f's ID is 0
    REQUIRE(query.constraints[0].symbol == f);
    REQUIRE(query.constraints[0].variables.size() == 1);
    REQUIRE(query.constraints[0].variables[0] == 0);

    // Second constraint: h(; 1) - h's ID is 1
    REQUIRE(query.constraints[1].symbol == h);
    REQUIRE(query.constraints[1].variables.size() == 1);
    REQUIRE(query.constraints[1].variables[0] == 1);

    // Third constraint: g(0, 1; 2) - children IDs first, then g's ID last
    REQUIRE(query.constraints[2].symbol == g);
    REQUIRE(query.constraints[2].variables.size() == 3);
    REQUIRE(query.constraints[2].variables[0] == 0); // f (first child)
    REQUIRE(query.constraints[2].variables[1] == 1); // h (second child)
    REQUIRE(query.constraints[2].variables[2] == 2); // g's ID (children < parent in post-order)

    // Head should contain the root variable (added last in compile())
    REQUIRE(query.head.size() == 1);
    REQUIRE(query.head[0] == 2); // g's ID
}

TEST_CASE("Deeply nested expression compilation", "[pattern_compiler]")
{
    // Create expression: add(mul(x, y), z) where x, y, z are variables
    Theory theory;
    auto add = theory.add_operator("add", 2);
    auto mul = theory.add_operator("mul", 2);

    Compiler compiler(theory);
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(add (mul ?x ?y) ?z)", "(add (mul ?x ?y) ?z)");
    auto [query, subst] = compiler.compile(rule);

    // Variable assignment order (POST-ORDER, children before parents):
    // x=0 (first var), y=1 (second var), mul=2, z=3 (third var), add=4 (root)
    // Constraints with IDs in LAST position: mul(0, 1; 2), add(2, 3; 4)
    REQUIRE(query.constraints.size() == 2);

    // mul(0, 1; 2) - x, y as args, then mul's ID last
    REQUIRE(query.constraints[0].symbol == mul);
    REQUIRE(query.constraints[0].variables.size() == 3);
    REQUIRE(query.constraints[0].variables[0] == 0); // x (first variable)
    REQUIRE(query.constraints[0].variables[1] == 1); // y (second variable)
    REQUIRE(query.constraints[0].variables[2] == 2); // mul's ID (children < parent in post-order)

    // add(2, 3; 4) - mul_id and z as args, then add's ID last
    REQUIRE(query.constraints[1].symbol == add);
    REQUIRE(query.constraints[1].variables.size() == 3);
    REQUIRE(query.constraints[1].variables[0] == 2); // mul_id (first arg)
    REQUIRE(query.constraints[1].variables[1] == 3); // z (third variable)
    REQUIRE(query.constraints[1].variables[2] == 4); // add's ID (root, highest ID in post-order)

    REQUIRE(query.head.size() == 4);
    REQUIRE(query.head[0] == 0); // x - added first
    REQUIRE(query.head[1] == 1); // y
    REQUIRE(query.head[2] == 3); // z
    REQUIRE(query.head[3] == 4); // root (add) - added last
}

TEST_CASE("Multiple patterns compilation", "[pattern_compiler]")
{
    Theory theory;
    Symbol f = theory.add_operator("f", 0);
    Symbol g = theory.add_operator("g", 0);

    RewriteRule rule1 = theory.add_rewrite_rule("rule1", "(f)", "(f)");
    RewriteRule rule2 = theory.add_rewrite_rule("rule2", "(g)", "(g)");
    Vec<RewriteRule> patterns = {rule1, rule2};

    Compiler compiler(theory);
    auto kernels = compiler.compile_many(patterns);

    REQUIRE(kernels.size() == 2);

    // First query: f(0)
    REQUIRE(kernels[0].first.constraints.size() == 1);
    REQUIRE(kernels[0].first.constraints[0].symbol == f);
    REQUIRE(kernels[0].first.head.size() == 1);
    REQUIRE(kernels[0].first.head[0] == 0);

    // Second query: g(0) - note that variable IDs reset for each pattern
    REQUIRE(kernels[1].first.constraints.size() == 1);
    REQUIRE(kernels[1].first.constraints[0].symbol == g);
    REQUIRE(kernels[1].first.head.size() == 1);
    REQUIRE(kernels[1].first.head[0] == 0);
}
