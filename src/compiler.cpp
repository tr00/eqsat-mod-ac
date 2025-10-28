#include "compiler.h"
#include "theory.h"
#include <utility>

HashMap<var_t, int> create_consecutive_index_map(const Vec<var_t>& unique_indices)
{
    HashMap<var_t, int> index_map;
    int consecutive_index = 0;

    for (int old_index : unique_indices)
    {
        index_map[old_index] = consecutive_index++;
    }

    return index_map;
}

Compiler::Compiler(const Theory& theory)
    : theory(theory)
    , next_id(0)
{
}

var_t Compiler::compile_rec(const std::shared_ptr<Expr>& expr, HashMap<Symbol, var_t>& symbol_to_var, Query& query)
{
    if (expr->is_variable())
    {
        auto it = symbol_to_var.find(expr->symbol);
        if (it != symbol_to_var.end()) return it->second;

        // create new var_t and add to map and head
        var_t id = next_id++;
        symbol_to_var[expr->symbol] = id;
        query.add_head_var(id);
        return id;
    }
    else
    {
        Vec<var_t> constraint_vars;

        // if the current expr is AC we want to insert the term id
        // into the constraint with term-id < children... < eclass-id
        if (theory.get_arity(expr->symbol) == AC)
        {
            var_t term_id = next_id++;
            constraint_vars.push_back(term_id);
        }

        for (const auto& child : expr->children)
        {
            var_t child_var = compile_rec(child, symbol_to_var, query);
            constraint_vars.push_back(child_var);
        }

        var_t eclass_id = next_id++;
        constraint_vars.push_back(eclass_id);

        query.add_constraint(expr->symbol, constraint_vars);

        return eclass_id;
    }
}

std::pair<Query, Subst> Compiler::compile(RewriteRule rule)
{
    // Reset variable counter for each compilation
    next_id = 0;

    // Symbol to variable mapping (for future extensions like variable reuse)
    HashMap<Symbol, var_t> env;

    // Create empty query with name
    Query query(rule.name);

    // Add the root variable to the head

    // Compile the pattern recursively
    var_t root = compile_rec(rule.lhs, env, query);
    query.add_head_var(root);

    HashMap<Symbol, int> env2;
    auto transl = create_consecutive_index_map(query.head);

    for (auto [sym, var] : env)
        env2[sym] = transl[var];

    Subst subst(rule.name, rule.rhs, env2, query.head.size());

    return std::pair(query, subst);
}

Vec<std::pair<Query, Subst>> Compiler::compile_many(const Vec<RewriteRule>& rules)
{
    Vec<std::pair<Query, Subst>> kernels;
    kernels.reserve(rules.size());

    for (const auto& rule : rules)
        kernels.push_back(compile(rule));

    return kernels;
}
