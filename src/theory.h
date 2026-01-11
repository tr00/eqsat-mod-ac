#pragma once

#include <memory>
#include <string>

#include "symbol_table.h"
#include "utils/vec.h"

namespace eqsat
{

// #define AC -1
constexpr auto AC = -1;

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
class Expr
{
  public:
    NodeKind kind;
    Symbol symbol;
    Vec<std::shared_ptr<Expr>> children;

    /**
     * @brief Creates a pattern variable expression.
     * @param var Symbol identifier for the variable name
     * @return Shared pointer to the created variable expression
     *
     * Example: auto x = Expression::make_variable(symbols.intern("x"));
     */
    static std::shared_ptr<Expr> make_variable(Symbol var);

    /**
     * @brief Creates a nullary operator expression (no children).
     * @param op Symbol identifier for the operator name
     * @return Shared pointer to the created operator expression
     *
     * Example: auto zero = Expression::make_operator(symbols.intern("0"));
     */
    static std::shared_ptr<Expr> make_operator(Symbol op);

    /**
     * @brief Creates an operator expression with child expressions.
     * @param op Symbol identifier for the operator name
     * @param children Vector of child expressions
     * @return Shared pointer to the created operator expression
     *
     * Example: auto sum = Expression::make_operator(op_sym, {x_expr, y_expr});
     */
    static std::shared_ptr<Expr> make_operator(Symbol op, const Vec<std::shared_ptr<Expr>>& children);

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

    size_t nchildren() const
    {
        return children.size();
    }

    /**
     * @brief Convert expression to S-expression string
     * @param symbols Symbol table for name resolution
     * @return S-expression representation
     */
    std::string to_sexpr(const SymbolTable& symbols) const;

    /**
     * @brief Check if the expression is a linear pattern
     *
     * A pattern is linear if each variable appears at most once.
     * Non-linear patterns like (f ?x ?x) are not currently supported.
     *
     * @return true if the pattern is linear, false otherwise
     */
    bool is_linear() const;

  private:
    /**
     * @brief Private constructor for creating expressions.
     * @param kind Type of expression (OPERATOR or VARIABLE)
     * @param sym Symbol identifier
     */
    Expr(NodeKind kind, Symbol sym);

    /**
     * @brief Private constructor for creating operator expressions with children.
     * @param kind Type of expression (should be OPERATOR)
     * @param op Symbol identifier for operator
     * @param children Vector of child expressions
     */
    Expr(NodeKind kind, Symbol op, const Vec<std::shared_ptr<Expr>>& children);
};

class RewriteRule
{
  public:
    Symbol name;
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    RewriteRule(Symbol name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs);
};

class Theory
{
  public:
    SymbolTable symbols;

    // symbol --> arity
    HashMap<Symbol, int> operators;
    Vec<RewriteRule> rewrite_rules;

    Theory()
    {
    }

    inline Symbol intern(const std::string& str)
    {
        return symbols.intern(str);
    }

    Symbol add_operator(const std::string& op, int arity)
    {
        return add_operator(intern(op), arity);
    }

    Symbol add_operator(Symbol symbol, int arity);

    /**
     * @brief Add an opaque operator with the given arity.
     * @param arity The arity of the operator (use AC for associative-commutative)
     * @return Symbol identifier for the opaque operator
     *
     * Opaque operators have unique IDs but return "<opaque>" when converted to string.
     * Useful for representing generated or anonymous operators.
     */
    Symbol add_opaque_operator(int arity);

    bool has_operator(Symbol symbol) const;
    int get_arity(Symbol symbol) const;

    RewriteRule add_rewrite_rule(const std::string& name, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs);
    RewriteRule add_rewrite_rule(const std::string& name, const std::string& lhs_str, const std::string& rhs_str);
};

} // namespace eqsat
