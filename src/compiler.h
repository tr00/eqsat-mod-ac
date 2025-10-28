/**
 * @brief Compiler for transforming rewrite rules into conjunctive queries and substitutions
 *
 * # Overview
 *
 * The Compiler translates pattern expressions (left-hand sides of rewrite rules) into
 * conjunctive queries that can be efficiently executed by the query engine. It also
 * generates substitution templates for instantiating the right-hand side with matched
 * variable bindings.
 *
 * # Architecture
 *
 * The compilation process consists of:
 * 1. **Pattern Analysis**: Traverse the pattern expression tree
 * 2. **Variable Assignment**: Assign unique variable IDs using post-order traversal
 * 3. **Constraint Generation**: Create database constraints from expression structure
 * 4. **Query Construction**: Build conjunctive query with proper variable ordering
 * 5. **Substitution Template**: Create template for RHS instantiation
 *
 * # Variable ID Assignment (Critical Invariant)
 *
 * Variables are assigned IDs in **post-order traversal** of the expression tree:
 * - **Children variable IDs < parent variable ID** (always)
 * - Root expression receives the highest ID
 * - Pattern variables (?x, ?y, etc.) get IDs when first encountered
 * - Each operator application gets an ID representing its e-class
 *
 * Example: `add(mul(?x, ?y), ?z)`
 * ```
 * Post-order: ?x(0) → ?y(1) → mul(2) → ?z(3) → add(4)
 * IDs: x=0, y=1, mul=2, z=3, add=4
 * ```
 *
 * This invariant is **critical** for:
 * - Ensuring queries execute in correct dependency order
 * - Enabling functional dependency optimizations
 * - Maintaining consistency with database tuple format
 *
 * # Constraint Format
 *
 * Generated constraints follow the database tuple format: `op(arg1, ..., argN; eclass_id)`
 * where **e-class ID is in the LAST position**.
 *
 * ## Standard Operators
 *
 * For `add(mul(?x, ?y), ?z)` with IDs: add=0, mul=1, x=2, y=3, z=4
 *
 * Generated constraints:
 * ```
 * mul(2, 3; 1)  // mul(?x, ?y) in e-class 1, children first, then result
 * add(1, 4; 0)  // add(mul, ?z) in e-class 0, children first, then result
 * ```
 *
 * ## AC Operators
 *
 * AC operators require special handling with an additional term-id variable:
 * ```
 * Pattern: mul_ac(?x, ?y)  [assuming mul_ac is AC]
 * IDs: mul_ac=0, term_id=1, x=2, y=3
 *
 * Constraint: mul_ac(0, 1, 2, 3)
 *            Format: (eclass_id, term_id, child1, child2, ...)
 * ```
 *
 * The term-id disambiguates different terms in the same e-class, necessary for
 * multiset-based matching in AC relations.
 *
 * # Query Head Construction
 *
 * The query head determines which variables are returned by query execution:
 * 1. Root variable (ID 0) is **always first** in the head
 * 2. Pattern variables (?x, ?y, etc.) follow in order of appearance
 *
 * Example: `add(mul(?x, ?y), ?z)`
 * ```
 * Head: [0, 2, 3, 4]  // root, x, y, z
 * ```
 *
 * After compilation, a consecutive index map is created to renormalize variable
 * IDs for the substitution environment, mapping sparse IDs to dense [0, 1, 2, ...].
 *
 * # Substitution Template
 *
 * The Subst object created alongside the query contains:
 * - **RHS expression**: The right-hand side expression tree
 * - **Environment**: Maps pattern variable symbols to head indices
 * - **Head size**: Number of variables in query result
 *
 * When a query match is found, the substitution uses this template to instantiate
 * the RHS with the matched variable bindings.
 *
 * # Compilation Process
 *
 * ## Single Rule Compilation: `compile(rule)`
 *
 * 1. Reset variable counter to 0
 * 2. Create empty query with rule name
 * 3. Add root variable (0) to query head
 * 4. Recursively compile LHS expression:
 *    - For pattern variables: Look up or create ID, add to head
 *    - For operators: Allocate eclass ID, compile children, generate constraint
 *    - For AC operators: Also allocate term ID
 * 5. Create consecutive index map for head variables
 * 6. Build substitution with RHS, environment, and head size
 * 7. Return (Query, Subst) pair
 *
 * ## Batch Compilation: `compile_many(rules)`
 *
 * Compiles multiple rules independently:
 * - Each rule gets its own variable ID space (reset between rules)
 * - Returns vector of (Query, Subst) pairs
 * - Efficient for compiling all rewrite rules in a theory
 *
 * # Invariants
 *
 * 1. **Variable Ordering**: Parent IDs < children IDs (pre-order traversal)
 * 2. **Constraint Format**: op(args...; eclass_id) with ID last
 * 3. **AC Format**: op(eclass_id, term_id, args...)
 * 4. **Root in Head**: Variable 0 always first in query head
 * 5. **Fresh Variables**: Each compilation starts with next_id = 0
 *
 * # Usage Example
 *
 * ```cpp
 * Theory theory;
 * Symbol add = theory.add_operator("add", 2);
 * Symbol mul = theory.add_operator("mul", 2);
 *
 * // Create rewrite rule: add(mul(?x, ?y), ?z) → mul(add(?x, ?z), add(?y, ?z))
 * RewriteRule rule = theory.add_rewrite_rule(
 *     "distributivity",
 *     "(add (mul ?x ?y) ?z)",
 *     "(mul (add ?x ?z) (add ?y ?z))"
 * );
 *
 * Compiler compiler(theory);
 * auto [query, subst] = compiler.compile(rule);
 *
 * // Query has 2 constraints:
 * // mul(2, 3; 1)  where x=2, y=3, mul=1
 * // add(1, 4; 0)  where mul=1, z=4, add=0
 *
 * // Query head: [0, 2, 3, 4]  (root, x, y, z)
 *
 * // Subst can instantiate RHS using matched values for x, y, z
 * ```
 *
 * # Implementation Notes
 *
 * - Variable IDs are allocated sequentially in pre-order traversal
 * - Pattern variables (?x) are memoized in environment to handle reuse
 * - Each operator node gets a unique e-class ID
 * - AC operators get an additional term ID for multiset indexing
 * - The consecutive index map densifies variable IDs for substitution
 * - Compiler is stateless between compilations (next_id reset each time)
 *
 * # Interaction with Query Engine
 *
 * The compiled queries are designed to work with the generic query engine:
 * - Variable ordering ensures dependencies are satisfied during execution
 * - Constraint format matches database tuple layout
 * - AC constraints work with multiset indices
 * - Query head specifies which variables to extract from matches
 *
 * The substitution object is used by the e-graph to instantiate new terms
 * when a pattern match is found during equality saturation.
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
