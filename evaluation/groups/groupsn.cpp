#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <random>

#include "egraph.h"
#include "enode.h"
#include "theory.h"

int main()
{
    size_t n = 200;

    // proves that: (+ x0 ... xn) = (+ x0 ... xn x0 (-x0))

    Theory theory;

    theory.add_operator("one", 0);
    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", AC);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");
    theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");

    // free variables
    Vec<std::shared_ptr<Expr>> args;
    args.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        auto sym = theory.add_opaque_operator(0);
        args[i] = Expr::make_operator(sym);
    }

    EGraph egraph(theory);

    Vec<id_t> children1(n);
    Vec<id_t> children2(n);

    for (size_t i = 0; i < n; ++i)
    {
        id_t id = egraph.add_expr(args[i]);

        children1[i] = id;
        children2[i] = id;
    }

    std::default_random_engine rng;

    std::shuffle(children1.begin(), children1.end(), rng);
    std::shuffle(children2.begin(), children2.end(), rng);

    children2.push_back(children2[0]);
    children2.push_back(egraph.add_enode(ENode{inv, {children2[0]}}));

    // print enodes
    std::cout << "1st enode: (mul ";
    for (size_t i = 0; i < n; ++i)
    {
        std::cout << children1[i];
        if (i < n - 1)
            std::cout << " ";
    }
    std::cout << ")\n";

    std::cout << "2nd enode: (mul ";
    for (size_t i = 0; i < n + 1; ++i)
        std::cout << children2[i] << " ";
    std::cout << "(inv " << children2[0] << "))\n";
    std::cout << std::flush;

    auto a = egraph.add_enode(ENode{mul, children1});
    auto b = egraph.add_enode(ENode{mul, children2});

    // --

    egraph.saturate(2);
    egraph.rebuild();

    // --

    bool res = egraph.is_equiv(a, b);
    std::cout << "result: " << (res ? "true" : "false") << std::endl;

    egraph.dump_to_file("dump_group.txt");

    return res ? 0 : 1;
}
