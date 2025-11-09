#include "egraph.h"
#include "theory.h"
#include <iostream>
#include <memory>

int main()
{
    size_t n = 3;
    assert(n >= 2);

    Theory theory;

    Symbol var = theory.add_operator("x", 0);
    Symbol one = theory.add_operator("1", 0);
    Symbol mul = theory.add_operator("*", AC);
    Symbol add = theory.add_operator("+", AC);

    theory.add_rewrite_rule("distr-1", "(+ (* ?x ?y) (* ?x ?z))", "(* ?x (+ ?z ?y))");
    theory.add_rewrite_rule("distr-2", "(* ?x (+ ?z ?y))", "(+ (* ?x ?y) (* ?x ?z))");

    Vec<Symbol> coeffs{
        theory.add_operator("a0", 0),
        theory.add_operator("a1", 0),
        theory.add_operator("a2", 0),
    };
    // coeffs.reserve(n);
    // for (size_t i = 0; i < n; ++i)
    //     coeffs.push_back(theory.add_opaque_operator(0));

    EGraph egraph(theory);

    // a0 + a1 x^1 + a2 x^2 + ... + an x^n
    Vec<std::shared_ptr<Expr>> children;
    children.push_back(Expr::make_operator(mul, {
                                                    Expr::make_operator(coeffs[0]),
                                                    Expr::make_operator(one),
                                                }));
    children.push_back(Expr::make_operator(mul, {
                                                    Expr::make_operator(coeffs[1]),
                                                    Expr::make_operator(var),
                                                }));
    for (size_t i = 2; i < n; ++i)
    {
        Vec<std::shared_ptr<Expr>> monomial{Expr::make_operator(coeffs[i])};
        monomial.reserve(i + 1);

        for (size_t j = 0; j < i; ++j)
            monomial.push_back(Expr::make_operator(var));

        children.push_back(Expr::make_operator(mul, monomial));
    }

    auto input = Expr::make_operator(add, children);
    std::cout << input->to_sexpr(theory.symbols) << std::endl;

    egraph.add_expr(input);

    egraph.saturate(2);

    egraph.dump_to_file("dump.txt");

    return 0;
}
