#include "compiler.h"
#include "egraph.h"
#include "symbol_table.h"
#include "theory.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("EGraph can handle rewrite rules with pattern compilation", "[egraph][rewrite]")
{
    // Create theory
    Theory theory;

    // Create operators
    Symbol zero_sym = theory.intern("0");
    Symbol one_sym = theory.intern("1");
    Symbol add_sym = theory.intern("+");
    Symbol mul_sym = theory.intern("*");

    // Add operators to theory
    theory.add_operator(zero_sym, 0);
    theory.add_operator(one_sym, 0);
    theory.add_operator(add_sym, 2);
    theory.add_operator(mul_sym, 2);

    SECTION("Add rewrite rules and compile patterns")
    {
        // Rule 1: 1 * x -> x (multiplicative identity)
        theory.add_rewrite_rule("mul-one", "(* (1) ?x)", "?x");

        // Rule 2: x * (y + z) -> (x * y) + (x * z) (distributivity)
        theory.add_rewrite_rule("distr", "(* ?x (+ ?y ?z))", "(+ (* ?x ?y) (* ?x ?z))");

        // Verify that the rules were added
        REQUIRE(theory.rewrite_rules.size() == 2);

        // Test pattern compilation
        Compiler compiler(theory);

        // Compile the left-hand side of the first rule
        auto [identity_query, identity_subst] = compiler.compile(theory.rewrite_rules[0]);
        REQUIRE(identity_query.constraints.size() == 2); // one constraint for "1", one for "*"
        REQUIRE(identity_query.head.size() > 0);

        // Compile the left-hand side of the distributivity rule
        auto [distributivity_query, distributivity_subst] = compiler.compile(theory.rewrite_rules[1]);
        REQUIRE(distributivity_query.constraints.size() == 2); // one for "+", one for "*"
        REQUIRE(distributivity_query.head.size() > 0);

        // Create EGraph with the theory containing rewrite rules
        EGraph egraph(theory);

        // Insert some concrete terms to test the structure
        auto zero_expr = Expr::make_operator(zero_sym);
        auto one_concrete = Expr::make_operator(one_sym);
        auto add_zero_one = Expr::make_operator(add_sym, {zero_expr, one_concrete});

        id_t zero_id = egraph.add_expr(zero_expr);
        id_t one_id = egraph.add_expr(one_concrete);
        id_t sum_id = egraph.add_expr(add_zero_one);

        // All should have different IDs initially
        REQUIRE(zero_id != one_id);
        REQUIRE(zero_id != sum_id);
        REQUIRE(one_id != sum_id);

        // Test that we can insert a term that matches the pattern structure
        // 1 * 0 should be insertable as a concrete term
        auto mul_one_zero = Expr::make_operator(mul_sym, {one_concrete, zero_expr});
        id_t mul_id = egraph.add_expr(mul_one_zero);
        REQUIRE(mul_id > 0);
    }

    SECTION("Compile multiple patterns in batch")
    {
        // Create multiple patterns using string-based interface
        theory.add_rewrite_rule("pattern1", "(* (1) ?x)", "?x");       // 1 * x -> x
        theory.add_rewrite_rule("pattern2", "(+ (0) ?x)", "?x");       // 0 + x -> x
        theory.add_rewrite_rule("pattern3", "(* ?x ?y)", "(* ?x ?y)"); // x * y -> x * y

        Vec<RewriteRule> patterns = {theory.rewrite_rules[theory.rewrite_rules.size() - 3],
                                     theory.rewrite_rules[theory.rewrite_rules.size() - 2],
                                     theory.rewrite_rules[theory.rewrite_rules.size() - 1]};

        // Compile all patterns
        Compiler compiler(theory);
        auto kernels = compiler.compile_many(patterns);

        REQUIRE(kernels.size() == 3);

        // Each query should have at least one constraint
        for (const auto& [query, subst] : kernels)
        {
            REQUIRE(query.constraints.size() >= 1);
            REQUIRE(query.head.size() > 0);
        }
    }
}
