#include "query.h"

// Constraint implementation
Constraint::Constraint(symbol_t op, const std::vector<var_t> &vars) : operator_symbol(op), variables(vars)
{
}

// Query implementation
Query::Query()
{
}

Query::Query(const std::vector<Constraint> &constraints, const std::vector<var_t> &head)
    : constraints(constraints), head(head)
{
}

void Query::add_constraint(const Constraint &constraint)
{
    constraints.push_back(constraint);
}

void Query::add_constraint(symbol_t op, const std::vector<var_t> &vars)
{
    constraints.emplace_back(op, vars);
}

void Query::add_head_var(var_t var)
{
    head.push_back(var);
}
