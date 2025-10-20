#include "../src/compiler.h"
#include "../src/symbol_table.h"
#include "../src/theory.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Simple expression compilation", "[pattern_compiler]")
{
    // Create a simple expression: f()
    Theory theory;
    Symbol f = theory.intern("f");

    Compiler compiler;
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(f)", "(f)");
    auto [query, subst] = compiler.compile(rule);

    // Should have one constraint: f(; 0) where 0 is the e-class ID (LAST position)
    REQUIRE(query.constraints.size() == 1);
    REQUIRE(query.constraints[0].operator_symbol == f);
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
    Symbol f = theory.intern("f");
    Symbol g = theory.intern("g");
    Symbol h = theory.intern("h");

    Compiler compiler;
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(g (f) (h))", "(g (f) (h))");
    auto [query, subst] = compiler.compile(rule);

    // Variable assignment order (PRE-ORDER): g=0, f=1, h=2 (parents before children)
    // Constraints with IDs in LAST position: f(; 1), h(; 2), g(1, 2; 0)
    REQUIRE(query.constraints.size() == 3);

    // First constraint: f(; 1) - f's ID is 1
    REQUIRE(query.constraints[0].operator_symbol == f);
    REQUIRE(query.constraints[0].variables.size() == 1);
    REQUIRE(query.constraints[0].variables[0] == 1);

    // Second constraint: h(; 2) - h's ID is 2
    REQUIRE(query.constraints[1].operator_symbol == h);
    REQUIRE(query.constraints[1].variables.size() == 1);
    REQUIRE(query.constraints[1].variables[0] == 2);

    // Third constraint: g(1, 2; 0) - children IDs first, then g's ID last
    REQUIRE(query.constraints[2].operator_symbol == g);
    REQUIRE(query.constraints[2].variables.size() == 3);
    REQUIRE(query.constraints[2].variables[0] == 1); // f (first child)
    REQUIRE(query.constraints[2].variables[1] == 2); // h (second child)
    REQUIRE(query.constraints[2].variables[2] == 0); // g's ID (parent < children in pre-order)

    // Head should contain the root variable (added first in compile())
    REQUIRE(query.head.size() == 1);
    REQUIRE(query.head[0] == 0); // g's ID
}

TEST_CASE("Deeply nested expression compilation", "[pattern_compiler]")
{
    // Create expression: add(mul(x, y), z) where x, y, z are variables
    Theory theory;
    Symbol add = theory.intern("add");
    Symbol mul = theory.intern("mul");

    Compiler compiler;
    RewriteRule rule = theory.add_rewrite_rule("test_rule", "(add (mul ?x ?y) ?z)", "(add (mul ?x ?y) ?z)");
    auto [query, subst] = compiler.compile(rule);

    // Variable assignment order (PRE-ORDER, parents before children):
    // add=0 (root), mul=1, x=2 (first var), y=3 (second var), z=4 (third var)
    // Constraints with IDs in LAST position: mul(2, 3; 1), add(1, 4; 0)
    REQUIRE(query.constraints.size() == 2);

    // mul(2, 3; 1) - x, y as args, then mul's ID last
    REQUIRE(query.constraints[0].operator_symbol == mul);
    REQUIRE(query.constraints[0].variables.size() == 3);
    REQUIRE(query.constraints[0].variables[0] == 2); // x (first variable)
    REQUIRE(query.constraints[0].variables[1] == 3); // y (second variable)
    REQUIRE(query.constraints[0].variables[2] == 1); // mul's ID (parent < children in pre-order)

    // add(1, 4; 0) - mul_id and z as args, then add's ID last
    REQUIRE(query.constraints[1].operator_symbol == add);
    REQUIRE(query.constraints[1].variables.size() == 3);
    REQUIRE(query.constraints[1].variables[0] == 1); // mul_id (first arg)
    REQUIRE(query.constraints[1].variables[1] == 4); // z (third variable)
    REQUIRE(query.constraints[1].variables[2] == 0); // add's ID (root, smallest ID in pre-order)

    REQUIRE(query.head.size() == 4);
    REQUIRE(query.head[0] == 0); // root (add) - added first
    REQUIRE(query.head[1] == 2); // x
    REQUIRE(query.head[2] == 3); // y
    REQUIRE(query.head[3] == 4); // z
}

TEST_CASE("Multiple patterns compilation", "[pattern_compiler]")
{
    Theory theory;
    Symbol f = theory.intern("f");
    Symbol g = theory.intern("g");

    RewriteRule rule1 = theory.add_rewrite_rule("rule1", "(f)", "(f)");
    RewriteRule rule2 = theory.add_rewrite_rule("rule2", "(g)", "(g)");
    Vec<RewriteRule> patterns = {rule1, rule2};

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
