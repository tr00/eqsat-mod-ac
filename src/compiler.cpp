#include "compiler.h"
#include "theory.h"
#include <unordered_map>
#include <vector>

std::unordered_map<int, int> create_consecutive_index_map(const std::vector<int>& unique_indices)
{
    std::unordered_map<int, int> index_map;
    int consecutive_index = 0;

    for (int old_index : unique_indices)
    {
        index_map[old_index] = consecutive_index++;
    }

    return index_map;
}

Compiler::Compiler() : next_id(0)
{
}

var_t Compiler::compile_rec(const std::shared_ptr<Expr>& expr, std::unordered_map<symbol_t, var_t>& symbol_to_var,
                            Query& query)
{
    if (expr->is_variable())
    {
        auto it = symbol_to_var.find(expr->symbol);
        if (it != symbol_to_var.end())
            return it->second;

        // create new var_t and add to map and head
        var_t id = next_id++;
        symbol_to_var[expr->symbol] = id;
        query.add_head_var(id);
        return id;
    }
    else
    {
        // Handle operators
        // Assign a new variable ID for this expression
        var_t id = next_id++;

        // Create constraint variables list: first is the expression's own variable
        std::vector<var_t> constraint_vars;
        constraint_vars.push_back(id);

        // Recursively compile children and add their variable IDs to constraint
        for (const auto& child : expr->children)
        {
            var_t child_var = compile_rec(child, symbol_to_var, query);
            constraint_vars.push_back(child_var);
        }

        // Create and add constraint for this expression
        query.add_constraint(expr->symbol, constraint_vars);

        return id;
    }
}

Query Compiler::compile(RewriteRule rule)
{
    // Reset variable counter for each compilation
    next_id = 0;

    // Symbol to variable mapping (for future extensions like variable reuse)
    std::unordered_map<symbol_t, var_t> env;

    // Create empty query with name
    Query query(rule.name);

    // Compile the pattern recursively
    var_t root = compile_rec(rule.lhs, env, query);

    // Add the root variable to the head
    query.add_head_var(root);

    return query;
}

std::vector<Query> Compiler::compile_many(const std::vector<RewriteRule>& rules)
{
    std::vector<Query> queries;
    queries.reserve(rules.size());

    for (const auto& rule : rules)
    {
        queries.push_back(compile(rule));
    }

    return queries;
}
