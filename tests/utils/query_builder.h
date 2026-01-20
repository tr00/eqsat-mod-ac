#pragma once

#include "query.h"
#include "theory.h"
#include "types.h"

namespace eqsat::test
{

/**
 * @brief Builder for constructing Query objects with a fluent interface
 *
 * Provides a clean, readable way to construct queries in tests without
 * verbose vector construction and manual method calls.
 *
 * Example usage:
 * @code
 * auto query = QueryBuilder(theory, theory.intern("my_query"))
 *     .with_constraint(add, {0, 1, 2})
 *     .with_constraint(mul, {2, 3, 4})
 *     .with_head_vars({0, 1, 2, 3, 4})
 *     .build();
 * @endcode
 */
class QueryBuilder
{
  private:
    Query query;

  public:
    /**
     * @brief Construct a new QueryBuilder with a query name
     *
     * @param name Symbol identifier for the query name
     */
    explicit QueryBuilder(Symbol name)
        : query(name)
    {
    }

    /**
     * @brief Construct a new QueryBuilder with a theory and string name
     *
     * @param theory Theory object to intern the name symbol
     * @param name String name for the query (will be interned)
     */
    QueryBuilder(Theory& theory, const std::string& name)
        : query(theory.intern(name))
    {
    }

    /**
     * @brief Add a constraint to the query
     *
     * @param op Operator symbol for the constraint
     * @param vars Vector of variable IDs for the constraint
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_constraint(Symbol op, const Vec<var_t>& vars)
    {
        query.add_constraint(op, vars);
        return *this;
    }

    /**
     * @brief Add a constraint to the query using initializer list
     *
     * @param op Operator symbol for the constraint
     * @param vars Initializer list of variable IDs for the constraint
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_constraint(Symbol op, std::initializer_list<var_t> vars)
    {
        query.add_constraint(op, Vec<var_t>(vars));
        return *this;
    }

    /**
     * @brief Add a pre-constructed constraint to the query
     *
     * @param constraint Constraint object to add
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_constraint(const Constraint& constraint)
    {
        query.add_constraint(constraint);
        return *this;
    }

    /**
     * @brief Add a single variable to the query head
     *
     * @param var Variable ID to add to the head
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_head_var(var_t var)
    {
        query.add_head_var(var);
        return *this;
    }

    /**
     * @brief Add multiple variables to the query head
     *
     * @param vars Vector of variable IDs to add to the head
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_head_vars(const Vec<var_t>& vars)
    {
        for (var_t var : vars)
        {
            query.add_head_var(var);
        }
        return *this;
    }

    /**
     * @brief Add multiple variables to the query head using initializer list
     *
     * @param vars Initializer list of variable IDs to add to the head
     * @return Reference to this builder for method chaining
     */
    QueryBuilder& with_head_vars(std::initializer_list<var_t> vars)
    {
        for (var_t var : vars)
        {
            query.add_head_var(var);
        }
        return *this;
    }

    /**
     * @brief Build and return the constructed Query
     *
     * @return The constructed Query object (moved)
     */
    Query build()
    {
        return std::move(query);
    }

    /**
     * @brief Get a reference to the underlying query (useful for inspection)
     *
     * @return Reference to the query being built
     */
    Query& get()
    {
        return query;
    }

    /**
     * @brief Get a const reference to the underlying query (useful for inspection)
     *
     * @return Const reference to the query being built
     */
    const Query& get() const
    {
        return query;
    }
};

} // namespace eqsat::test
