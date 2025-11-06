#include <iostream>

#include "egraph.h"
#include "theory.h"

int main()
{
    size_t n = 5;

    Theory theory;

    auto var = theory.add_operator("v", 0);
    auto mul = theory.add_operator("*", AC);
    auto add = theory.add_operator("+", AC);

    // x*y + x*z = x * (y + z)
    theory.add_rewrite_rule("distr-1", "(+ (* ?x ?y) (* ?x ?z))", "(* ?x (+ ?z ?y))");
    theory.add_rewrite_rule("distr-2", "(* ?x (+ ?z ?y))", "(+ (* ?x ?y) (* ?x ?z))");

    Vec<std::shared_ptr<Expr>> vars(n);
    for (size_t i = 0; i < n; ++i)
        vars[i] = Expr::make_operator(theory.add_opaque_operator(0));

    auto h1 = Expr::make_operator(add, {});
    for (size_t i = 0; i < n; ++i)
    {
        auto hi = Expr::make_operator(mul, {var, vars[i]});
        h1->children.push_back(hi);
    }

    // h(v0) * h(v1) * ... * h(vn-1)
    auto h2 = Expr::make_operator(mul, {var, Expr::make_operator(add, vars)});

    EGraph egraph(theory);

    auto a = egraph.add_expr(h1);
    auto b = egraph.add_expr(h2);

    egraph.saturate(2);

    bool res = egraph.is_equiv(a, b);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    egraph.dump_to_file("dump_ring.txt");

    return res ? 0 : 1;
}
