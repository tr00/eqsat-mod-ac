#include <memory>
#include <stdexcept>

#include "parser.h"
#include "theory.h"

namespace eqsat
{

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

Symbol Theory::add_opaque_operator(int arity)
{
    Symbol symbol = symbols.create_opaque();
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
    // Check if the LHS pattern is linear
    if (!lhs->is_linear())
    {
        throw std::invalid_argument("Non-linear pattern in rule '" + name + "': " + lhs->to_sexpr(symbols) +
                                    "\nEach variable must appear at most once in the pattern. " +
                                    "Non-linear patterns like (f ?x ?x) are not currently supported.");
    }

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
    if (is_variable())
        return "?" + symbols.get_string(symbol);

    std::string result = "(" + symbols.get_string(symbol);

    for (const auto& child : children)
        result += " " + child->to_sexpr(symbols);

    result += ")";
    return result;
}

bool Expr::is_linear() const
{
    // A pattern is non-linear if the same variable appears multiple times
    // as a DIRECT child of any operator.
    //
    // Example: (mul ?x ?x) is non-linear because ?x appears twice as direct children
    // Example: (mul ?x (inv ?x)) is LINEAR because the second ?x is nested in inv
    //
    // We need to check each operator node and see if any variable appears more than once
    // among its direct children.

    std::function<bool(const Expr *)> check_linear = [&](const Expr *expr) -> bool {
        if (expr->is_variable())
        {
            return true; // A single variable is always linear
        }

        // Check direct children for duplicate variables
        HashMap<Symbol, size_t> direct_child_vars;
        for (const auto& child : expr->children)
        {
            if (child->is_variable())
            {
                direct_child_vars[child->symbol]++;
                if (direct_child_vars[child->symbol] > 1)
                {
                    return false; // Same variable appears twice as direct child
                }
            }
        }

        // Recursively check all children
        for (const auto& child : expr->children)
        {
            if (!check_linear(child.get()))
            {
                return false;
            }
        }

        return true;
    };

    return check_linear(this);
}

} // namespace eqsat
