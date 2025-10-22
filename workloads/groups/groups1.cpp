#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    Theory theory; // abelian group

    theory.add_operator("one", 0);
    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", AC);

    // free variable
    auto a = theory.add_operator("a", 0);

    theory.add_rewrite_rule("id", "(mul ?x (one))", "?x");
    theory.add_rewrite_rule("inv", "(mul ?x (inv ?x))", "(one)");

    auto expr_a = Expr::make_operator(a);
    auto expr_ia = Expr::make_operator(inv, {expr_a});
    auto expr_iia = Expr::make_operator(inv, {expr_ia});
    auto expr_iia_a_ia = Expr::make_operator(mul, {expr_iia, expr_a, expr_ia});

    EGraph egraph(theory);

    auto a_id = egraph.add_expr(expr_a);
    auto iia_id = egraph.add_expr(expr_iia);

    // insert critical pair required for the proof
    egraph.add_expr(expr_iia_a_ia);

    egraph.saturate(5);

    bool res = egraph.is_equiv(a_id, iia_id);

    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    return 0;
}
