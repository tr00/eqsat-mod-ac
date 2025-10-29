/**
 * @brief Compiler for transforming rewrite rules into conjunctive queries and substitutions
 *
 * Translates pattern expressions (LHS of rewrite rules) into conjunctive queries for the query
 * engine, and generates substitution templates for instantiating the RHS with matched bindings.
 *
 * # Variable ID Assignment
 *
 * Variables are assigned IDs in **post-order traversal** of the expression tree:
 * - Children IDs < parent IDs (ensures correct dependency order)
 * - Root expression receives highest ID
 * - Pattern variables (?x, ?y) get IDs when first encountered
 *
 * Example: `add(mul(?x, ?y), ?z)` → IDs: x=0, y=1, mul=2, z=3, add=4
 *
 * # Constraint Format
 *
 * Constraints follow database tuple format: `op(arg1, ..., argN; eclass_id)`
 * where e-class ID is in the **LAST position**.
 *
 * **Standard operators**: `mul(x, y; result_id)` - children first, then result
 * **AC operators**: `mul_ac(result_id, term_id, x, y, ...)` - result and term-id first, then children
 *
 * The term-id for AC operators disambiguates different terms in the same e-class.
 *
 * # Query Head Construction
 *
 * The query head determines which variables are returned:
 * 1. Pattern variables (?x, ?y, ...) added during traversal
 * 2. Root variable (LHS result) **always last**
 *
 * Example: `add(mul(?x, ?y), ?z)` → Head: [0, 1, 3, 4] (x, y, z, root)
 *
 * After compilation, a consecutive index map renormalizes sparse variable IDs to dense [0, 1, 2, ...]
 * for the substitution environment.
 *
 * # Substitution Template
 *
 * The Subst object contains:
 * - RHS expression tree
 * - Environment mapping pattern variable symbols to head indices
 * - Head size (number of variables in query result)
 *
 * When a match is found, the substitution instantiates the RHS with matched variable bindings.
 *
 * # Compilation Process
 *
 * **Single rule** (`compile(rule)`):
 * 1. Reset variable counter to 0
 * 2. Recursively compile LHS:
 *    - Pattern variables: look up or create ID, add to head
 *    - Operators: allocate eclass ID, compile children, generate constraint
 *    - AC operators: also allocate term ID
 * 3. Add root variable to head (last position)
 * 4. Create consecutive index map and build substitution
 *
 * **Batch** (`compile_many(rules)`):
 * - Each rule gets independent variable ID space
 * - Returns vector of (Query, Subst) pairs
 *
 * # Invariants
 *
 * 1. **Variable Ordering**: Parent IDs > children IDs (post-order)
 * 2. **Constraint Format**: Standard: `op(args...; id)`, AC: `op(id, term_id, args...)`
 * 3. **Root in Head**: Root variable always last in query head
 * 4. **Fresh Variables**: Each compilation starts with next_id = 0
 *
 * # Implementation Notes
 *
 * - Pattern variables are memoized to handle reuse
 * - Each operator node gets unique e-class ID
 * - Compiler is stateless between compilations
 * - Compiled queries designed for generic query engine execution
 * - Substitution used by e-graph during equality saturation
 */

#pragma once

#include <memory>

#include "query.h"
#include "theory.h"

class Compiler
{
  private:
    const Theory& theory;
    var_t next_id;

    var_t compile_rec(const std::shared_ptr<Expr>& expr, HashMap<Symbol, var_t>& env, Query& query);

  public:
    Compiler(const Theory& theory);

    std::pair<Query, Subst> compile(RewriteRule rule);

    Vec<std::pair<Query, Subst>> compile_many(const Vec<RewriteRule>& rules);
};
