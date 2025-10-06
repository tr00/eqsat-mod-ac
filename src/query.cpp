#include <algorithm>

#include "permutation.h"
#include "query.h"

// Constraint implementation
Constraint::Constraint(Symbol op, const Vec<var_t>& vars) : operator_symbol(op), variables(vars)
{
    // Compute permutation: map from current positions to sorted positions
    // This represents the relative ordering of variables

    // Create pairs of (value, original_index)
    Vec<std::pair<var_t, uint32_t>> indexed_vars;
    indexed_vars.reserve(vars.size());
    for (size_t i = 0; i < vars.size(); ++i)
    {
        indexed_vars.push_back({vars[i], static_cast<uint32_t>(i)});
    }

    // Sort by value to get the sorted order
    std::sort(indexed_vars.begin(), indexed_vars.end());

    // Build the permutation: for each position, what's its rank in sorted order?
    Vec<uint32_t> perm(vars.size());
    for (size_t sorted_pos = 0; sorted_pos < indexed_vars.size(); ++sorted_pos)
    {
        uint32_t original_pos = indexed_vars[sorted_pos].second;
        perm[original_pos] = static_cast<uint32_t>(sorted_pos);
    }

    // Convert permutation to index
    permutation = permutation_to_index(perm);
}

Query::Query(Symbol name) : name(name)
{
}

Query::Query(Symbol name, const Vec<Constraint>& constraints, const Vec<var_t>& head)
    : name(name), constraints(constraints), head(head)
{
}

void Query::add_constraint(const Constraint& constraint)
{
    constraints.push_back(constraint);
}

void Query::add_constraint(Symbol op, const Vec<var_t>& vars)
{
    constraints.emplace_back(op, vars);
    var_t maxvar = *std::max_element(vars.begin(), vars.end());
    nvars = std::max(nvars, maxvar);
}

void Query::add_head_var(var_t var)
{
    head.push_back(var);
}

id_t Subst::instantiate(callback_t f, const Vec<id_t>& match)
{
    return instantiate_rec(f, match, root);
}

id_t Subst::instantiate_rec(callback_t f, const Vec<id_t>& match, std::shared_ptr<Expr> expr)
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

std::string Query::to_string(const SymbolTable& symbols) const
{
    std::string result = "Query " + symbols.get_string(name) + ":\n";
    result += "  Constraints:\n";
    for (const auto& constraint : constraints)
    {
        result += "    " + symbols.get_string(constraint.operator_symbol) + "(";
        for (size_t i = 0; i < constraint.variables.size(); ++i)
        {
            if (i > 0)
                result += ", ";
            result += "v" + std::to_string(constraint.variables[i]);
        }
        result += ") [perm=" + std::to_string(constraint.permutation) + "]\n";
    }
    result += "  Head: [";
    for (size_t i = 0; i < head.size(); ++i)
    {
        if (i > 0)
            result += ", ";
        result += "v" + std::to_string(head[i]);
    }
    result += "]\n";
    return result;
}
