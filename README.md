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

## Implementation Status

**✅ COMPLETE**: The rewrite engine is now fully functional!

### Completed Features

- ✅ **Theory & Expression parsing** - Define operators and rewrite rules
- ✅ **Database engine** - Relations, indices (trie-based), set operations
- ✅ **Union-Find** - Tracks equivalence classes
- ✅ **E-graph** - Term insertion, hash-consing, unification
- ✅ **Query system** - Conjunctive queries with constraints
- ✅ **Pattern compilation** - Converts rewrite rules to queries + substitutions
- ✅ **Constraint hashing** - Enables efficient constraint lookups
- ✅ **Query execution engine** - VM-based execution with backtracking
- ✅ **Match instantiation** - Builds RHS terms and unifies with LHS
- ✅ **Saturation loop** - Iteratively applies rewrite rules until fixpoint

### Architecture Overview

```
Theory (rewrite rules)
    ↓
Compiler (patterns → queries + substitutions)
    ↓
EGraph.saturate()
    ↓
Engine.execute() → matches
    ↓
Subst.instantiate() + unify() → new equivalences
```

### Key Implementation Details

1. **Pattern Compilation**:
   - LHS pattern → Query (conjunctive constraints)
   - RHS pattern → Subst (substitution environment)
   - Head variables track bindings to instantiate

2. **Query Execution**:
   - Returns flat `Vec<id_t>` with all matches
   - Each match is `head_size` elements
   - Uses trie indices for efficient filtering

3. **Match Instantiation**:
   - Callback mechanism to add enodes during instantiation
   - LHS root (first match element) unified with instantiated RHS
   - Helper function `apply_matches()` encapsulates the logic

### All Tests Passing ✅

- Unit tests: Compiler, Union-Find, Trie indices, Sets, Permutations
- System tests: E-graph operations, Database, Rewrite rules
