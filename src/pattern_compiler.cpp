#include "pattern_compiler.h"

PatternCompiler::PatternCompiler() : next_var_id(0) {}

var_t PatternCompiler::compile_expression_recursive(
    const std::shared_ptr<Expression>& expr,
    std::unordered_map<symbol_t, var_t>& symbol_to_var,
    Query& query
) {
    if (expr->is_variable()) {
        // Handle pattern variables
        auto it = symbol_to_var.find(expr->symbol);
        if (it != symbol_to_var.end()) {
            // Variable already seen, return existing var_t
            return it->second;
        } else {
            // New variable, create new var_t and add to map and head
            var_t var_id = next_var_id++;
            symbol_to_var[expr->symbol] = var_id;
            query.add_head_var(var_id);
            return var_id;
        }
    } else {
        // Handle operators
        // Assign a new variable ID for this expression
        var_t id = next_var_id++;

        // Create constraint variables list: first is the expression's own variable
        std::vector<var_t> constraint_vars;
        constraint_vars.push_back(id);

        // Recursively compile children and add their variable IDs to constraint
        for (const auto& child : expr->children) {
            var_t child_var = compile_expression_recursive(child, symbol_to_var, query);
            constraint_vars.push_back(child_var);
        }

        // Create and add constraint for this expression
        query.add_constraint(expr->symbol, constraint_vars);

        return id;
    }
}

Query PatternCompiler::compile_pattern(const std::shared_ptr<Expression>& pattern) {
    // Reset variable counter for each compilation
    next_var_id = 0;

    // Symbol to variable mapping (for future extensions like variable reuse)
    std::unordered_map<symbol_t, var_t> symbol_to_var;

    // Create empty query
    Query query;

    // Compile the pattern recursively
    var_t root = compile_expression_recursive(pattern, symbol_to_var, query);

    // Add the root variable to the head
    query.add_head_var(root);

    return query;
}

std::vector<Query> PatternCompiler::compile_patterns(const std::vector<std::shared_ptr<Expression>>& patterns) {
    std::vector<Query> queries;
    queries.reserve(patterns.size());

    for (const auto& pattern : patterns) {
        queries.push_back(compile_pattern(pattern));
    }

    return queries;
}
