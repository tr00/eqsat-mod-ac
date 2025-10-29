#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    Theory theory;

    // free variable
    auto var = theory.add_operator("var", 0);

    theory.add_operator("one", 0);
    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", AC);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");
    theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");

    EGraph egraph(theory);

    auto var_expr = Expr::make_operator(var);
    auto mul_expr = Expr::make_operator(mul, {var_expr, var_expr, Expr::make_operator(inv, {var_expr})});

    auto var_id = egraph.add_expr(var_expr);
    auto mul_id = egraph.add_expr(mul_expr);

    egraph.saturate(1);
    egraph.rebuild();
    egraph.saturate(1);

    // auto one_id = egraph.add_expr(Expr::make_operator(one));

    bool res = egraph.is_equiv(var_id, mul_id);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    egraph.dump_to_file();
}
