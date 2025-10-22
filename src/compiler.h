#pragma once

#include <memory>

#include "query.h"
#include "theory.h"

class Compiler
{
  private:
    const Theory& theory;
    var_t next_id;

    var_t compile_rec(const std::shared_ptr<Expr>& expr, HashMap<Symbol, var_t>& env, Query& query);

  public:
    Compiler(const Theory& theory);

    std::pair<Query, Subst> compile(RewriteRule rule);

    Vec<std::pair<Query, Subst>> compile_many(const Vec<RewriteRule>& rules);
};
