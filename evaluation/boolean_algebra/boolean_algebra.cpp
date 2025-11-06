#include "iostream"

#include "egraph.h"
#include "theory.h"

int main()
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto tt = theory.add_operator("true", 0);
    theory.add_operator("false", 0);
    auto andd = theory.add_operator("and", AC);
    theory.add_operator("or", AC);

    theory.add_rewrite_rule("and_true", "(and ?x (true))", "?x");
    theory.add_rewrite_rule("and_false", "(and ?x (false))", "(false)");
    theory.add_rewrite_rule("or_true", "(or ?x (true))", "(true)");
    theory.add_rewrite_rule("or_false", "(or ?x (false))", "?x");

    EGraph egraph(theory);

    auto a_expr = Expr::make_operator(a);
    auto b_expr = Expr::make_operator(b);
    auto tt_expr = Expr::make_operator(tt);

    // and(a, b, true)
    auto and_expr = Expr::make_operator(andd, {a_expr, b_expr, tt_expr});
    id_t and_id = egraph.add_expr(and_expr);

    egraph.saturate(2);

    auto and_ab = Expr::make_operator(andd, {a_expr, b_expr});
    id_t and_ab_id = egraph.add_expr(and_ab);

    bool res = egraph.is_equiv(and_id, and_ab_id);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    // egraph.dump_to_file();

    return res ? 0 : 1;
}
