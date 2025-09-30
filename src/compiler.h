#pragma once

#include "query.h"
#include "theory.h"
#include <memory>

class Compiler
{
  private:
    var_t next_id;

    var_t compile_rec(const std::shared_ptr<Expr>& expr, HashMap<Symbol, var_t>& env, Query& query);

  public:
    Compiler();

    std::pair<Query, Subst> compile(RewriteRule rule);

    Vec<std::pair<Query, Subst>> compile_many(const Vec<RewriteRule>& rules);
};
