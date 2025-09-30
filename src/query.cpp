#include "query.h"
#include <vector>

// Constraint implementation
Constraint::Constraint(Symbol op, const std::vector<var_t>& vars) : operator_symbol(op), variables(vars)
{
}

Query::Query(Symbol name) : name(name)
{
}

Query::Query(Symbol name, const std::vector<Constraint>& constraints, const std::vector<var_t>& head)
    : name(name), constraints(constraints), head(head)
{
}

void Query::add_constraint(const Constraint& constraint)
{
    constraints.push_back(constraint);
}

void Query::add_constraint(Symbol op, const std::vector<var_t>& vars)
{
    constraints.emplace_back(op, vars);
}

void Query::add_head_var(var_t var)
{
    head.push_back(var);
}

id_t Subst::instantiate(callback_t f, const std::vector<id_t>& match)
{
    return instantiate_rec(f, match, root);
}

id_t Subst::instantiate_rec(callback_t f, const std::vector<id_t>& match, std::shared_ptr<Expr> expr)
{
    if (expr->is_variable())
        return match[env[expr->symbol]];

    Vec<id_t> children;
    children.reserve(expr->nchildren());

    for (auto child : expr->children)
    {
        id_t child_id = instantiate_rec(f, match, child);
        children.push_back(child_id);
    }

    return f(expr->symbol, std::move(children));
}
