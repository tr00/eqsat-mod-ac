#include "theory.h"
#include <memory>
#include <vector>

Expression::Expression(NodeKind k, symbol_t sym) : kind(k), symbol(sym) {}

Expression::Expression(NodeKind k, symbol_t op, const std::vector<std::shared_ptr<Expression>>& children)
    : kind(k), symbol(op), children(children) {}

std::shared_ptr<Expression> Expression::make_variable(symbol_t var_name) {
    return std::shared_ptr<Expression>(new Expression(NodeKind::VARIABLE, var_name));
}

std::shared_ptr<Expression> Expression::make_operator(symbol_t op) {
    return std::shared_ptr<Expression>(new Expression(NodeKind::OPERATOR, op));
}

std::shared_ptr<Expression> Expression::make_operator(symbol_t op, const std::vector<std::shared_ptr<Expression>>& children) {
    return std::shared_ptr<Expression>(new Expression(NodeKind::OPERATOR, op, children));
}

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
