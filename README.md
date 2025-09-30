# Equality Saturation Modulo AC

This project implements equality saturation on top of a small custom built relational database engine.
Terms are stored as tuples in relations and pattern matching is reduced to answering conjunctive queries.

We implement a special purpose virtual machine, just for answering these kinds of queries efficiently.
Each query is compiled to a sequence of nested iterations and set intersections.

### Example

```cpp
Theory theory;

auto one = theory.add_operator("1", 0);
auto add = theory.add_operator("+", 2);
auto mul = theory.add_operator("*", 2);

theory.add_rewrite_rule("identity", "(* 1 ?x)", "?x");

Egraph egraph(theory);

egraph.saturate(/* iterations: */ 10);
```

---

## TODO: 6-Hour Sprint to Working System

**Goal**: Enable adding rewrite rules, running the engine, and getting back matching tuples.

### Critical Path (Must Complete)

1. **Implement Engine::execute() return value [1.5 hours]** - `src/engine.h`, `src/engine.cpp`
   - Change signature: `void execute()` → `std::vector<std::vector<id_t>> execute()`
   - Return all matching tuples (variable bindings)
   - Query execution VM needs to collect matches during iteration

2. **Implement match instantiation [1.5 hours]** - `src/egraph.cpp:78-88`
   - For each match, call `subst.instantiate()` to build RHS term
   - Insert instantiated term into e-graph via `add_expr()`
   - Unify LHS root with RHS result: `unify(lhs_root, rhs_root)`
   - LHS root = first variable in query head

3. **Integration & testing [1.5 hours]**
   - Fix any remaining compilation errors
   - Write simple integration test: insert terms, add rule, saturate, check equivalences
   - Debug execution flow end-to-end

### Implementation Notes

- **Query head**: Should contain LHS root variable as first element for unification
- **Callback in instantiate**: Pass lambda that calls `egraph.add_enode(symbol, children)`
- **Database indices**: Already built in `saturate()` loop - no changes needed
- **Union-Find**: Already tracks equivalences - just need to call `unify()` with results

### Current Status

- ✅ Theory, Expression, Signature parsing
- ✅ Database, relations, indices, sets
- ✅ Union-Find for equivalence classes
- ✅ E-graph term insertion and hash-consing
- ✅ Query data structures
- ✅ Pattern compilation to queries (complete)
- ✅ **Constraint hashing** - done
- ✅ **Compiler return type** - returns Query+Subst pairs
- ✅ **Query/Subst storage** - stored in EGraph
- ⚠️ **Engine execution** - returns void, needs to return matches
- ⚠️ **Match instantiation** - skeleton code only

### Files Requiring Changes

1. `src/engine.h` - Change execute() signature
2. `src/engine.cpp` - Collect and return matches
3. `src/egraph.cpp` - Implement instantiation loop

**Time Budget**: ~3 hours remaining for core implementation + testing
