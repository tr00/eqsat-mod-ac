#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    std::cout << "E-Graph Simple Pattern Matching Demo\n";
    std::cout << "=====================================\n\n";

    Theory theory;

    auto one = theory.add_operator("one", 0);
    auto var = theory.add_operator("var", 0);
    auto mul = theory.add_operator("mul", 2);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    std::cout << "Created theory with operators:\n";
    std::cout << "  - one (constant)\n";
    std::cout << "  - var (variable)\n";
    std::cout << "  - mul (binary, associative-commutative)\n\n";

    EGraph egraph(theory);

    std::cout << "Created e-graph\n\n";

    // build expression: 1 * v
    auto var_expr = Expr::make_operator(var);
    auto one_expr = Expr::make_operator(one);
    auto mul_expr = Expr::make_operator(mul, {var_expr, one_expr});

    std::cout << "Building expressions:\n";
    std::cout << "  var_expr = var\n";
    std::cout << "  one_expr = one\n";
    std::cout << "  mul_expr = mul(one, var)\n\n";

    // insert expressions into e-graph
    auto var_id = egraph.add_expr(var_expr);
    auto one_id = egraph.add_expr(one_expr);
    auto mul_id = egraph.add_expr(mul_expr);

    std::cout << "Inserted expressions into e-graph:\n";
    std::cout << "  var_id = " << var_id << "\n";
    std::cout << "  one_id = " << one_id << "\n";
    std::cout << "  mul_id = " << mul_id << "\n";
    std::cout << "\n";

    bool equiv_before = egraph.is_equiv(var_id, mul_id);
    std::cout << "Before saturation:\n";
    std::cout << "  is_equiv(var_id, mul_id) = " << (equiv_before ? "true" : "false") << "\n\n";

    std::cout << "Running saturation (depth = 1)...\n";
    egraph.saturate(1);
    std::cout << "Saturation complete\n\n";

    bool equiv_after = egraph.is_equiv(var_id, mul_id);
    std::cout << "After saturation:\n";
    std::cout << "  is_equiv(var_id, mul_id) = " << (equiv_after ? "true" : "false") << "\n\n";

    std::cout << "\nDemo finished successfully!\n";

    return 0;
}
