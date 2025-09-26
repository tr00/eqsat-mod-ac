#pragma once

#include "query.h"
#include "theory.h"
#include <memory>
#include <unordered_map>

class Compiler
{
  private:
    var_t next_id;

    var_t compile_rec(const std::shared_ptr<Expr>& expr, std::unordered_map<symbol_t, var_t>& symbol_to_var,
                      Query& query);

  public:
    Compiler();

    Query compile(RewriteRule rule);

    std::vector<Query> compile_many(const std::vector<RewriteRule>& rules);
};
