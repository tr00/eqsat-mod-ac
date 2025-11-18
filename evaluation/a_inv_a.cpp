#include "compiler.h"
#include "egraph.h"
#include <iostream>

int main()
{
    Theory theory;

    auto var = theory.add_operator("var", 0);
    auto one = theory.add_operator("one", 0);
    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", AC);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");
    theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");

    // Debug: print compiled queries
    Compiler compiler(theory);
    for (const auto& rule : theory.rewrite_rules)
    {
        auto [query, subst] = compiler.compile(rule);
        std::cout << query.to_string(theory.symbols) << std::endl;
    }

    EGraph egraph(theory);

    size_t n = 10;

    // 1 * a * a * ... * a
    // auto expr = Expr::make_operator(mul, {Expr::make_operator(one)});
    auto expr = Expr::make_operator(mul, {});
    (void)one;

    for (size_t i = 0; i < n; ++i)
        expr->children.push_back(Expr::make_operator(var));

    // <expr> * a^-1 * ... * a^-1
    for (size_t i = 0; i < n; ++i)
        expr->children.push_back(Expr::make_operator(inv, {Expr::make_operator(var)}));

    egraph.add_expr(expr);
    egraph.saturate(10);

    egraph.dump_to_file("dump_ainva.txt");

    return 0;
}
