#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "symbol_table.h"

/**
 * @brief Enumeration for distinguishing between different kinds of expression nodes.
 */
enum class NodeKind
{
    OPERATOR,
    VARIABLE
};

/**
 * @brief Represents an expression in the algebraic structure.
 *
 * An Expression can be either:
 * - An operator/function application with zero or more child expressions
 * - A pattern variable used in rewrite rules and pattern matching
 *
 * Examples:
 * - Nullary operator: f()
 * - Binary operator: add(x, y)
 * - Pattern variable: x
 * - Nested expression: mul(add(a, b), c)
 *
 * @note Use the static factory methods make_variable() and make_operator()
 *       instead of constructors to create Expression instances.
 */
class Expression
{
  public:
    NodeKind kind;
    symbol_t symbol;
    std::vector<std::shared_ptr<Expression>> children;

    /**
     * @brief Creates a pattern variable expression.
     * @param var Symbol identifier for the variable name
     * @return Shared pointer to the created variable expression
     *
     * Example: auto x = Expression::make_variable(symbols.intern("x"));
     */
    static std::shared_ptr<Expression> make_variable(symbol_t var);

    /**
     * @brief Creates a nullary operator expression (no children).
     * @param op Symbol identifier for the operator name
     * @return Shared pointer to the created operator expression
     *
     * Example: auto zero = Expression::make_operator(symbols.intern("0"));
     */
    static std::shared_ptr<Expression> make_operator(symbol_t op);

    /**
     * @brief Creates an operator expression with child expressions.
     * @param op Symbol identifier for the operator name
     * @param children Vector of child expressions
     * @return Shared pointer to the created operator expression
     *
     * Example: auto sum = Expression::make_operator(op_sym, {x_expr, y_expr});
     */
    static std::shared_ptr<Expression> make_operator(symbol_t op,
                                                     const std::vector<std::shared_ptr<Expression>> &children);

    /**
     * @brief Checks if this expression is a pattern variable.
     * @return true if this is a variable, false otherwise
     */
    bool is_variable() const
    {
        return kind == NodeKind::VARIABLE;
    }

    /**
     * @brief Checks if this expression is an operator application.
     * @return true if this is an operator, false otherwise
     */
    bool is_operator() const
    {
        return kind == NodeKind::OPERATOR;
    }

  private:
    /**
     * @brief Private constructor for creating expressions.
     * @param kind Type of expression (OPERATOR or VARIABLE)
     * @param sym Symbol identifier
     */
    Expression(NodeKind kind, symbol_t sym);

    /**
     * @brief Private constructor for creating operator expressions with children.
     * @param kind Type of expression (should be OPERATOR)
     * @param op Symbol identifier for operator
     * @param children Vector of child expressions
     */
    Expression(NodeKind kind, symbol_t op, const std::vector<std::shared_ptr<Expression>> &children);
};

class Signature
{
  public:
    std::unordered_map<symbol_t, int> operators;

    void add_operator(symbol_t symbol, int arity);
    bool has_operator(symbol_t symbol) const;
    int get_arity(symbol_t symbol) const;
};

class RewriteRule
{
  public:
    std::shared_ptr<Expression> left_side;
    std::shared_ptr<Expression> right_side;

    RewriteRule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs);
};

class Theory
{
  public:
    Signature signature;
    std::vector<RewriteRule> rewrite_rules;

    void add_operator(symbol_t symbol, int arity);
    void add_rewrite_rule(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs);
};
