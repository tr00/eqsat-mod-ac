#include "theory.h"
#include <memory>
#include <vector>

Expr::Expr(NodeKind k, symbol_t sym) : kind(k), symbol(sym)
{
}

Expr::Expr(NodeKind k, symbol_t op, const std::vector<std::shared_ptr<Expr>>& children)
    : kind(k), symbol(op), children(children)
{
}

std::shared_ptr<Expr> Expr::make_variable(symbol_t var_name)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::VARIABLE, var_name));
}

std::shared_ptr<Expr> Expr::make_operator(symbol_t op)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::OPERATOR, op));
}

std::shared_ptr<Expr> Expr::make_operator(symbol_t op, const std::vector<std::shared_ptr<Expr>>& children)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::OPERATOR, op, children));
}

RewriteRule::RewriteRule(symbol_t name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
    : name(name), lhs(lhs), rhs(rhs)
{
}

symbol_t Theory::add_operator(symbol_t symbol, int arity)
{
    operators[symbol] = arity;
    return symbol;
}

bool Theory::has_operator(symbol_t symbol) const
{
    return operators.find(symbol) != operators.end();
}

int Theory::get_arity(symbol_t symbol) const
{
    auto it = operators.find(symbol);
    if (it != operators.end())
    {
        return it->second;
    }
    return -1;
}

void Theory::add_rewrite_rule(const std::string& name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
{
    rewrite_rules.emplace_back(intern(name), lhs, rhs);
}
