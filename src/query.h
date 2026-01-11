#pragma once

#include <functional>
#include <memory>

#include "id.h"
#include "symbol_table.h"
#include "theory.h"
#include "utils/hash.h"

namespace eqsat
{

using namespace std;

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
    Symbol symbol;

    uint32_t permutation;

    /** @brief List of variables participating in this constraint */
    Vec<var_t> variables;

    /**
     * @brief Construct a new constraint
     *
     * @param op The operator symbol
     * @param vars Vector of variables for this constraint
     */
    Constraint(Symbol op, const Vec<var_t>& vars);

    /**
     * @brief Construct a new constraint with explicit permutation
     *
     * @param op The operator symbol
     * @param vars Vector of variables for this constraint
     * @param perm Explicit permutation value
     */
    Constraint(Symbol op, const Vec<var_t>& vars, uint32_t perm);

    bool operator==(const Constraint& other) const
    {
        return symbol == other.symbol && variables == other.variables;
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
    Vec<Constraint> constraints;

    /** @brief Variables to return in query results (projection) */
    Vec<var_t> head;

    var_t nvars = 0;

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
    Query(Symbol name, const Vec<Constraint>& constraints, const Vec<var_t>& head);

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
    void add_constraint(Symbol op, const Vec<var_t>& vars);

    /**
     * @brief Add a variable to the query head (result projection)
     *
     * @param var Variable to add to the head
     */
    void add_head_var(var_t var);

    /**
     * @brief Convert query to string for debugging
     * @param symbols Symbol table for name resolution
     * @return String representation of the query
     */
    std::string to_string(const SymbolTable& symbols) const;

    /**
     * @brief Get all required indices for this query
     * @return Vector of (operator_symbol, permutation) pairs needed for execution
     */
    Vec<std::pair<Symbol, uint32_t>> get_required_indices() const;
};

using callback_t = function<id_t(Symbol, Vec<id_t>)>;
class Subst
{
  public:
    Symbol name;
    size_t head_size;
    shared_ptr<Expr> root;
    HashMap<Symbol, int> env;

    id_t instantiate_rec(callback_t f, const Vec<id_t>& match, shared_ptr<Expr> expr);

    Subst(Symbol name, shared_ptr<Expr> root, HashMap<Symbol, int> env, size_t head_size)
        : name(name)
        , head_size(head_size)
        , root(root)
        , env(env)
    {
    }

    id_t instantiate(callback_t f, const Vec<id_t>& match);
};

} // namespace eqsat

namespace std
{
template <>
struct hash<eqsat::Constraint>
{
    size_t operator()(const eqsat::Constraint& constraint) const
    {
        uint64_t h1 = SEED;
        uint64_t h2 = eqsat::hash64(constraint.symbol);

        for (const auto& var : constraint.variables)
            h1 = eqsat::mix64(h1, var);

        return eqsat::mix64(h1, h2);
    }
};
} // namespace std
