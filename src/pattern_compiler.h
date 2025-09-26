#pragma once

#include "query.h"
#include "theory.h"
#include <memory>
#include <unordered_map>

class PatternCompiler
{
  private:
    var_t next_id;

    var_t compile_expression_rec(const std::shared_ptr<Expr>& expr, std::unordered_map<symbol_t, var_t>& symbol_to_var,
                                 Query& query);

  public:
    PatternCompiler();

    Query compile_pattern(const std::shared_ptr<Expr>& pattern);

    std::vector<Query> compile_patterns(const std::vector<std::shared_ptr<Expr>>& patterns);
};
