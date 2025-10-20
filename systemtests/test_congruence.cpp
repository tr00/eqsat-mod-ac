#include <catch2/catch_test_macros.hpp>

#include "egraph.h"
#include "theory.h"

TEST_CASE("EGraph proves fa = fb via congruence of a=b", "[egraph][rewrite][congruence]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto f = theory.add_operator("f", 1);

    EGraph egraph(theory);

    auto a_expr = Expr::make_operator(a);
    auto b_expr = Expr::make_operator(b);

    auto a_id = egraph.add_expr(a_expr);
    auto b_id = egraph.add_expr(b_expr);

    auto f_a = egraph.add_expr(Expr::make_operator(f, {a_expr}));
    auto f_b = egraph.add_expr(Expr::make_operator(f, {b_expr}));

    egraph.unify(a_id, b_id);

    REQUIRE(egraph.is_equiv(f_a, f_b) == false);

    egraph.rebuild();

    REQUIRE(egraph.is_equiv(f_a, f_b) == true);
}

TEST_CASE("EGraph proves g(f(a)) = g(f(b)) via congruence of a=b", "[egraph][rewrite][congruence]")
{
    Theory theory;

    auto a = theory.add_operator("a", 0);
    auto b = theory.add_operator("b", 0);
    auto f = theory.add_operator("f", 1);
    auto g = theory.add_operator("g", 1);

    EGraph egraph(theory);

    auto a_expr = Expr::make_operator(a);
    auto b_expr = Expr::make_operator(b);
    auto f_a_expr = Expr::make_operator(f, {a_expr});
    auto f_b_expr = Expr::make_operator(f, {b_expr});
    auto g_f_a_expr = Expr::make_operator(g, {f_a_expr});
    auto g_f_b_expr = Expr::make_operator(g, {f_b_expr});

    auto a_id = egraph.add_expr(a_expr);
    auto b_id = egraph.add_expr(b_expr);

    auto gfa = egraph.add_expr(g_f_a_expr);
    auto gfb = egraph.add_expr(g_f_b_expr);

    egraph.unify(a_id, b_id);

    REQUIRE(egraph.is_equiv(gfa, gfb) == false);

    egraph.rebuild();
    egraph.rebuild();

    REQUIRE(egraph.is_equiv(gfa, gfb) == true);
}
