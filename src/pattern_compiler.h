#pragma once

#include <memory>
#include <unordered_map>
#include "theory.h"
#include "query.h"

class PatternCompiler {
private:
    var_t next_var_id;
    
    // Helper function for recursive compilation
    var_t compile_expression_recursive(
        const std::shared_ptr<Expression>& expr,
        std::unordered_map<symbol_t, var_t>& symbol_to_var,
        Query& query
    );
    
public:
    PatternCompiler();
    
    // Main compilation function
    Query compile_pattern(const std::shared_ptr<Expression>& pattern);
    
    // Compile multiple patterns
    std::vector<Query> compile_patterns(const std::vector<std::shared_ptr<Expression>>& patterns);
};