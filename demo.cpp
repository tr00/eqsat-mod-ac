#include "egraph.h"
#include "symbol_table.h"
#include "theory.h"
#include <iostream>

int main()
{
    std::cout << "E-Graph Simple Pattern Matching Demo\n";
    std::cout << "=====================================\n\n";

    // Create theory
    Theory theory;

    // Add operators: one (0-ary), var (0-ary), mul (binary with AC properties)
    auto one = theory.add_operator("one", 0);
    auto var = theory.add_operator("var", 0);
    auto mul = theory.add_operator("mul", AC);

    std::cout << "Created theory with operators:\n";
    std::cout << "  - one (constant)\n";
    std::cout << "  - var (variable)\n";
    std::cout << "  - mul (binary, associative-commutative)\n\n";

    // Create EGraph with the theory
    EGraph egraph(theory);

    std::cout << "Created e-graph\n\n";

    // Build expressions: 1 * v
    auto var_expr = Expr::make_operator(var);
    auto one_expr = Expr::make_operator(one);
    auto mul_expr = Expr::make_operator(mul, {one_expr, var_expr});

    std::cout << "Building expressions:\n";
    std::cout << "  var_expr = var\n";
    std::cout << "  one_expr = one\n";
    std::cout << "  mul_expr = mul(one, var)\n\n";

    // Insert expressions into e-graph
    auto var_id = egraph.add_expr(var_expr);
    auto mul_id = egraph.add_expr(mul_expr);

    std::cout << "Inserted expressions into e-graph:\n";
    std::cout << "  var_id = " << var_id << "\n";
    std::cout << "  mul_id = " << mul_id << "\n\n";

    // Check if they are equivalent before saturation
    bool equiv_before = egraph.is_equiv(var_id, mul_id);
    std::cout << "Before saturation:\n";
    std::cout << "  is_equiv(var_id, mul_id) = " << (equiv_before ? "true" : "false") << "\n\n";

    // Run saturation with depth 1
    std::cout << "Running saturation (depth = 1)...\n";
    egraph.saturate(2);
    std::cout << "Saturation complete\n\n";

    std::cout << "Demo finished successfully!\n";

    return 0;
}
