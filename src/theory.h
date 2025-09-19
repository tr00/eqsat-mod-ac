#pragma once

#include <map>
#include <memory>
#include <vector>

#include "symbol_table.h"

class Expression {
public:
    symbol_t operator_symbol;
    std::vector<std::shared_ptr<Expression>> children;
    
    Expression(symbol_t op);
    Expression(symbol_t op, const std::vector<std::shared_ptr<Expression>>& children);
};

class Signature {
public:
    std::map<symbol_t, int> operators;
    
    void add_operator(symbol_t symbol, int arity);
    bool has_operator(symbol_t symbol) const;
    int get_arity(symbol_t symbol) const;
};

class RewriteRule {
public:
    std::shared_ptr<Expression> left_side;
    std::shared_ptr<Expression> right_side;
    
    RewriteRule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs);
};

class Theory {
public:
    Signature signature;
    std::vector<RewriteRule> rewrite_rules;
    
    void add_operator(symbol_t symbol, int arity);
    void add_rewrite_rule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs);
};