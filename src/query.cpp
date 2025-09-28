#include "query.h"
#include "egraph.h"
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

id_t Subst::instantiate(EGraph& egraph, const std::vector<id_t>& match)
{
    return instantiate_rec(egraph, match, root);
}

id_t Subst::instantiate_rec(EGraph& egraph, const std::vector<id_t>& match, std::shared_ptr<Expr> expr)
{
    if (expr->is_variable())
        return match[env[expr->symbol]];

    std::vector<id_t> children;
    children.reserve(expr->nchildren());

    for (auto child : expr->children)
    {
        id_t child_id = instantiate_rec(egraph, match, child);
        children.push_back(child_id);
    }

    return egraph.add_enode(expr->symbol, std::move(children));
}
