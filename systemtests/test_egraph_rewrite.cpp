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
    symbol_t zero_sym = theory.intern("0");
    symbol_t one_sym = theory.intern("1");
    symbol_t add_sym = theory.intern("+");
    symbol_t mul_sym = theory.intern("*");

    // Add operators to theory
    theory.add_operator(zero_sym, 0);
    theory.add_operator(one_sym, 0);
    theory.add_operator(add_sym, 2);
    theory.add_operator(mul_sym, 2);

    SECTION("Add rewrite rules and compile patterns")
    {
        // Create pattern variables
        symbol_t x_sym = theory.intern("x");
        symbol_t y_sym = theory.intern("y");
        symbol_t z_sym = theory.intern("z");

        auto x_var = Expr::make_variable(x_sym);
        auto y_var = Expr::make_variable(y_sym);
        auto z_var = Expr::make_variable(z_sym);

        // Rule 1: 1 * x -> x (multiplicative identity)
        auto one_expr = Expr::make_operator(one_sym);
        auto mul_one_x = Expr::make_operator(mul_sym, {one_expr, x_var});
        theory.add_rewrite_rule("mul-one", mul_one_x, x_var);

        // Rule 2: x * (y + z) -> (x * y) + (x * z) (distributivity)
        auto y_plus_z = Expr::make_operator(add_sym, {y_var, z_var});
        auto x_mul_sum = Expr::make_operator(mul_sym, {x_var, y_plus_z});
        auto x_mul_y = Expr::make_operator(mul_sym, {x_var, y_var});
        auto x_mul_z = Expr::make_operator(mul_sym, {x_var, z_var});
        auto distributed = Expr::make_operator(add_sym, {x_mul_y, x_mul_z});
        theory.add_rewrite_rule("distr", x_mul_sum, distributed);

        // Verify that the rules were added
        REQUIRE(theory.rewrite_rules.size() == 2);

        // Test pattern compilation
        Compiler compiler;

        // Compile the left-hand side of the first rule
        symbol_t rule_name = theory.intern("identity_test");
        RewriteRule identity_rule(rule_name, mul_one_x, x_var);
        Query identity_query = compiler.compile(identity_rule);
        REQUIRE(identity_query.constraints.size() == 2); // one constraint for "1", one for "*"
        REQUIRE(identity_query.head.size() > 0);

        // Compile the left-hand side of the distributivity rule
        symbol_t dist_rule_name = theory.intern("dist_test");
        RewriteRule dist_rule(dist_rule_name, x_mul_sum, distributed);
        Query distributivity_query = compiler.compile(dist_rule);
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
        // Create pattern variables
        symbol_t x_sym = theory.intern("x");
        symbol_t y_sym = theory.intern("y");
        auto x_var = Expr::make_variable(x_sym);
        auto y_var = Expr::make_variable(y_sym);

        // Create multiple patterns
        auto one_expr = Expr::make_operator(one_sym);
        auto zero_expr = Expr::make_operator(zero_sym);

        auto pattern1 = Expr::make_operator(mul_sym, {one_expr, x_var});  // 1 * x
        auto pattern2 = Expr::make_operator(add_sym, {zero_expr, x_var}); // 0 + x
        auto pattern3 = Expr::make_operator(mul_sym, {x_var, y_var});     // x * y

        symbol_t name1 = theory.intern("pattern1");
        symbol_t name2 = theory.intern("pattern2");
        symbol_t name3 = theory.intern("pattern3");
        std::vector<RewriteRule> patterns = {RewriteRule(name1, pattern1, x_var), RewriteRule(name2, pattern2, x_var),
                                             RewriteRule(name3, pattern3, pattern3)};

        // Compile all patterns
        Compiler compiler;
        std::vector<Query> queries = compiler.compile_many(patterns);

        REQUIRE(queries.size() == 3);

        // Each query should have at least one constraint
        for (const auto& query : queries)
        {
            REQUIRE(query.constraints.size() >= 1);
            REQUIRE(query.head.size() > 0);
        }
    }
}
