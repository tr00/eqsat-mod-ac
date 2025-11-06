#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    size_t n = 5;

    Theory theory;

    auto m = theory.add_operator("*", AC);
    auto h = theory.add_operator("h", 1);

    // h(x * y) = h(x) * h(y)
    theory.add_rewrite_rule("endo-1", "(h (* ?x ?y))", "(* (h ?x) (h ?y))");
    theory.add_rewrite_rule("endo-2", "(* (h ?x) (h ?y))", "(h (* ?x ?y))");

    Vec<std::shared_ptr<Expr>> vars(n);
    for (size_t i = 0; i < n; ++i)
        vars[i] = Expr::make_operator(theory.add_opaque_operator(0));

    // h(v0 * v1 * ... * vn-1)
    auto h1 = Expr::make_operator(h, {Expr::make_operator(m, vars)});

    // h(v0) * h(v1) * ... * h(vn-1)
    auto h2 = Expr::make_operator(m, {});
    for (size_t i = 0; i < n; ++i)
    {
        auto hi = Expr::make_operator(h, {vars[i]});
        h2->children.push_back(hi);
    }

    EGraph egraph(theory);

    auto a = egraph.add_expr(h1);
    auto b = egraph.add_expr(h2);

    egraph.saturate(4);
    // egraph.rebuild();

    bool res = egraph.is_equiv(a, b);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    egraph.dump_to_file("dump_endo.txt");

    return res ? 0 : 1;
}
