#include <memory>

#include "parser.h"
#include "theory.h"

Expr::Expr(NodeKind k, Symbol sym)
    : kind(k)
    , symbol(sym)
{
}

Expr::Expr(NodeKind k, Symbol op, const Vec<std::shared_ptr<Expr>>& children)
    : kind(k)
    , symbol(op)
    , children(children)
{
}

std::shared_ptr<Expr> Expr::make_variable(Symbol var_name)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::VARIABLE, var_name));
}

std::shared_ptr<Expr> Expr::make_operator(Symbol op)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::OPERATOR, op));
}

std::shared_ptr<Expr> Expr::make_operator(Symbol op, const Vec<std::shared_ptr<Expr>>& children)
{
    return std::shared_ptr<Expr>(new Expr(NodeKind::OPERATOR, op, children));
}

RewriteRule::RewriteRule(Symbol name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
    : name(name)
    , lhs(lhs)
    , rhs(rhs)
{
}

Symbol Theory::add_operator(Symbol symbol, int arity)
{
    operators[symbol] = arity;
    return symbol;
}

bool Theory::has_operator(Symbol symbol) const
{
    return operators.find(symbol) != operators.end();
}

int Theory::get_arity(Symbol symbol) const
{
    auto it = operators.find(symbol);
    if (it != operators.end())
    {
        return it->second;
    }
    return -1;
}

RewriteRule Theory::add_rewrite_rule(const std::string& name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
{
    RewriteRule rule(intern(name), lhs, rhs);
    rewrite_rules.push_back(rule);
    return rule;
}

RewriteRule Theory::add_rewrite_rule(const std::string& name, const std::string& lhs_str, const std::string& rhs_str)
{
    Parser parser(symbols);
    auto lhs = parser.parse_sexpr(lhs_str);
    auto rhs = parser.parse_sexpr(rhs_str);
    return add_rewrite_rule(name, lhs, rhs);
}

std::string Expr::to_sexpr(const SymbolTable& symbols) const
{
    if (is_variable()) return "?" + symbols.get_string(symbol);

    std::string result = "(" + symbols.get_string(symbol);

    for (const auto& child : children)
        result += " " + child->to_sexpr(symbols);

    result += ")";
    return result;
}
