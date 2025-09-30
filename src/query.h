#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "id.h"
#include "symbol_table.h"
#include "theory.h"

/**
 * @brief Variable type for query variables
 *
 * Represents variables in conjunctive queries using 32-bit unsigned integers.
 */
using var_t = uint32_t;

/**
 * @brief A constraint in a conjunctive query
 *
 * Represents a single constraint of the form operator_symbol(var1, var2, ..., varN).
 * For example, the constraint add(x, y, z) would have operator_symbol = "add"
 * and variables = [x, y, z].
 */
class Constraint
{
  public:
    /** @brief The operator symbol for this constraint */
    Symbol operator_symbol;

    /** @brief List of variables participating in this constraint */
    std::vector<var_t> variables;

    /**
     * @brief Construct a new constraint
     *
     * @param op The operator symbol
     * @param vars Vector of variables for this constraint
     */
    Constraint(Symbol op, const std::vector<var_t>& vars);

    bool operator==(const Constraint& other) const
    {
        return operator_symbol == other.operator_symbol && variables == other.variables;
    }
};

/**
 * @brief A conjunctive query for pattern matching
 *
 * Represents a conjunctive query consisting of multiple constraints and a head.
 * The query finds all variable bindings that satisfy all constraints simultaneously.
 *
 * Example query: find all x such that add(x, 1, y) AND mul(y, 2, z)
 * - constraints: [add(x, 1, y), mul(y, 2, z)]
 * - head: [x] (the variables we want to return)
 */
class Query
{
  public:
    /** @brief Name identifier for this query */
    Symbol name;

    /** @brief List of constraints that must all be satisfied */
    std::vector<Constraint> constraints;

    /** @brief Variables to return in query results (projection) */
    std::vector<var_t> head;

    /**
     * @brief Construct an empty query with a name
     *
     * @param name Name identifier for this query
     */
    Query(Symbol name);

    /**
     * @brief Construct a query with name, constraints and head variables
     *
     * @param name Name identifier for this query
     * @param constraints List of constraints for this query
     * @param head List of head variables to project in results
     */
    Query(Symbol name, const std::vector<Constraint>& constraints, const std::vector<var_t>& head);

    /**
     * @brief Add a constraint to this query
     *
     * @param constraint The constraint to add
     */
    void add_constraint(const Constraint& constraint);

    /**
     * @brief Add a constraint to this query
     *
     * @param op Operator symbol for the constraint
     * @param vars Variables for the constraint
     */
    void add_constraint(Symbol op, const std::vector<var_t>& vars);

    /**
     * @brief Add a variable to the query head (result projection)
     *
     * @param var Variable to add to the head
     */
    void add_head_var(var_t var);
};

namespace std
{
template <>
struct hash<Constraint>
{
    size_t operator()(const Constraint& constraint) const
    {
        size_t h1 = std::hash<Symbol>{}(constraint.operator_symbol);
        size_t h2 = 0;

        for (const auto& var : constraint.variables)
        {
            h2 ^= std::hash<var_t>{}(var) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
        }
        return h1 ^ (h2 << 1);
    }
};
} // namespace std

using callback_t = std::function<id_t(Symbol, Vec<id_t>)>;
class Subst
{
  private:
    Symbol name;
    std::shared_ptr<Expr> root;
    std::unordered_map<Symbol, int> env;

    id_t instantiate_rec(callback_t f, const std::vector<id_t>& match, std::shared_ptr<Expr> expr);

  public:
    Subst(Symbol name, std::shared_ptr<Expr> root, std::unordered_map<Symbol, int> env)
        : name(name), root(root), env(env)
    {
    }

    id_t instantiate(callback_t f, const std::vector<id_t>& match);
};
