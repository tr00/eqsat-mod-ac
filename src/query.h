#pragma once

#include "symbol_table.h"
#include <vector>

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
    symbol_t operator_symbol;

    /** @brief List of variables participating in this constraint */
    std::vector<var_t> variables;

    /**
     * @brief Construct a new constraint
     *
     * @param op The operator symbol
     * @param vars Vector of variables for this constraint
     */
    Constraint(symbol_t op, const std::vector<var_t>& vars);
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
    symbol_t name;

    /** @brief List of constraints that must all be satisfied */
    std::vector<Constraint> constraints;

    /** @brief Variables to return in query results (projection) */
    std::vector<var_t> head;

    /**
     * @brief Construct an empty query with a name
     *
     * @param name Name identifier for this query
     */
    Query(symbol_t name);

    /**
     * @brief Construct a query with name, constraints and head variables
     *
     * @param name Name identifier for this query
     * @param constraints List of constraints for this query
     * @param head List of head variables to project in results
     */
    Query(symbol_t name, const std::vector<Constraint>& constraints, const std::vector<var_t>& head);

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
    void add_constraint(symbol_t op, const std::vector<var_t>& vars);

    /**
     * @brief Add a variable to the query head (result projection)
     *
     * @param var Variable to add to the head
     */
    void add_head_var(var_t var);
};

class Subst
{
    symbol_t name;
};

/**
 *
 */
class QueryPlan
{
  private:
    symbol_t name;
    size_t arity;

  public:
    void execute();
};
