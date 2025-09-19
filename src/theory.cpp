#include "theory.h"

Expression::Expression(symbol_t op) : operator_symbol(op) {}

Expression::Expression(symbol_t op, const std::vector<std::shared_ptr<Expression>>& children)
    : operator_symbol(op), children(children) {}

void Signature::add_operator(symbol_t symbol, int arity) {
    operators[symbol] = arity;
}

bool Signature::has_operator(symbol_t symbol) const {
    return operators.find(symbol) != operators.end();
}

int Signature::get_arity(symbol_t symbol) const {
    auto it = operators.find(symbol);
    if (it != operators.end()) {
        return it->second;
    }
    return -1;
}

RewriteRule::RewriteRule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs)
    : left_side(lhs), right_side(rhs) {}

void Theory::add_operator(symbol_t symbol, int arity) {
    signature.add_operator(symbol, arity);
}

void Theory::add_rewrite_rule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) {
    rewrite_rules.emplace_back(lhs, rhs);
}