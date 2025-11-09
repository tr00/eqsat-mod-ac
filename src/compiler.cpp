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
    , next_term_id(0)
{
}

int Compiler::count_ac_operators(const std::shared_ptr<Expr>& expr) const
{
    if (expr->is_variable())
        return 0;

    int count = theory.get_arity(expr->symbol) == AC ? 1 : 0;

    for (const auto& child : expr->children)
        count += count_ac_operators(child);

    return count;
}

var_t Compiler::compile_rec(const std::shared_ptr<Expr>& expr, HashMap<Symbol, var_t>& symbol_to_var, Query& query)
{
    if (expr->is_variable())
    {
        auto it = symbol_to_var.find(expr->symbol);
        if (it != symbol_to_var.end())
            return it->second;

        // Pattern variables get IDs starting from offset (after all AC term-ids)
        var_t id = next_id++;
        symbol_to_var[expr->symbol] = id;
        query.add_head_var(id);
        return id;
    }
    else
    {
        Vec<var_t> constraint_vars;

        // AC term-ids get IDs from a separate counter (0, 1, 2, ...)
        // This ensures ALL term-ids < ALL pattern variables and eclass-ids
        if (theory.get_arity(expr->symbol) == AC)
        {
            var_t term_id = next_term_id++;
            constraint_vars.push_back(term_id);
        }

        for (const auto& child : expr->children)
        {
            var_t child_var = compile_rec(child, symbol_to_var, query);
            constraint_vars.push_back(child_var);
        }

        // E-class IDs also use next_id (come after AC term-ids)
        var_t eclass_id = next_id++;
        constraint_vars.push_back(eclass_id);

        // For AC constraints, use explicit permutation value to mark them
        // This prevents FD optimization with TrieIndex for AC operators
        if (theory.get_arity(expr->symbol) == AC)
        {
            query.add_constraint(Constraint(expr->symbol, constraint_vars, static_cast<uint32_t>(AC)));
        }
        else
        {
            query.add_constraint(Constraint(expr->symbol, constraint_vars));
        }

        return eclass_id;
    }
}

std::pair<Query, Subst> Compiler::compile(RewriteRule rule)
{
    // Pass 1: Count AC operators to reserve term-id slots
    int num_ac_ops = count_ac_operators(rule.lhs);

    // Pass 2: Compile with proper ID assignment
    // - AC term-ids get IDs 0, 1, 2, ..., (num_ac_ops - 1)
    // - Pattern variables and e-class IDs start at num_ac_ops
    next_term_id = 0;
    next_id = num_ac_ops;

    // Symbol to variable mapping (for pattern variable reuse)
    HashMap<Symbol, var_t> env;

    // Create empty query with name
    Query query(rule.name);

    // Compile the pattern recursively
    var_t root = compile_rec(rule.lhs, env, query);
    query.add_head_var(root);

    HashMap<Symbol, int> env2;
    auto transl = create_consecutive_index_map(query.head);

    for (const auto [sym, var] : env)
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
