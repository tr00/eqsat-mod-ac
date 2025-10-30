#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto andd = theory.add_operator("and", AC);

    theory.add_rewrite_rule("and_idem", "(and ?x ?x)", "?x");

    EGraph egraph(theory);

    auto a_expr = Expr::make_operator(a);

    // and(a, a, a, a, a) with idempotent rule
    // Expected: rule should fire multiple times to reduce and(a*5) -> a
    auto and_expr = Expr::make_operator(andd, {
                                                  a_expr,
                                                  a_expr,
                                                  a_expr,
                                                  a_expr,
                                              });

    id_t a_id = egraph.add_expr(a_expr);
    id_t and_id = egraph.add_expr(and_expr);

    egraph.saturate(1);

    bool res = egraph.is_equiv(a_id, and_id);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    egraph.dump_to_file();

    return 0;
}
