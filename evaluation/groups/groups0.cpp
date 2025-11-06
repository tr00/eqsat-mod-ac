#include "iostream"

#include "egraph.h"
#include "theory.h"

int main()
{
    Theory theory;

    auto var = theory.add_operator("var", 0);
    auto one = theory.add_operator("one", 0);
    auto mul = theory.add_operator("mul", AC);

    // Rewrite rule: mul(?x, one) -> ?x
    // Q(x, r) :- mul(t, x, y, r), one(y)
    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");

    EGraph egraph(theory);

    auto var_expr = Expr::make_operator(var);
    auto one_expr = Expr::make_operator(one);
    auto mul_expr = Expr::make_operator(mul, {var_expr, one_expr});

    id_t var_id = egraph.add_expr(var_expr);
    id_t mul_id = egraph.add_expr(mul_expr);

    egraph.saturate(1);

    bool res = egraph.is_equiv(var_id, mul_id) == true;
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    // egraph.dump_to_file();

    // (+ ?x (- ?x)) in? (+ a a (-a))
}
