# Equality Saturation Modulo AC

C++17 library implementing e-graphs (equality saturation) with custom database for conjunctive queries.

## Build System

- **Build**: `just build` (creates `libeqsat.a`)
- **Test**: `just {test/unittests/systemtests}`
- **Format**: `just format`
- **Full**: `just all`

## Core Components

### Basic Types (`src/id.h`)

- `id_t`, `symbol_t`, `var_t`: 32-bit identifiers
- `Vec<T>`: Alias for `gch::small_vector<T>` (optimized vector)
- `HashMap<K, V>`: Alias for `ankerl::unordered_dense::map<K, V>` (fast hash map)

### Symbol Management (`src/symbol_table.h`)

- `SymbolTable`: Thread-safe string ↔ symbol_t mapping

### Theory (`src/theory.h`)

- `Expression`: AST nodes for algebraic expressions
- `Signature`: Operator definitions
- `RewriteRule`: LHS → RHS transformations
- `Theory`: Operators and rewrite rules collection

### Database (`src/database.h`, `src/relations/`, `src/indices/`)

- `Database`: Named relation container
- `AbstractRelation`: Base relation interface
  - `RowStore`: Row-oriented relation storage
- `AbstractIndex`: Type-erased index wrapper
  - `TrieIndex`: Trie-based indexing for relations

**Important**: Relations store tuples as `op(arg1, arg2, ..., argN; eclass_id)` where the last column is the e-class identifier, NOT the result of the operation. For example, `add(1, 2; 10)` means that `add(1,2)` belongs to e-class 10.

### Set Operations (`src/sets/`)

- `AbstractSet`: Set wrapper with tagged-union polymorphism
  - `SortedVecSet`: Vector-based sorted set implementation
  - `SortedIterSet`: Iterator-based sorted set implementation

### Union-Find (`src/union_find.h`)

- `UnionFind`: Disjoint sets for equivalence classes

### E-Graph (`src/egraph.h`)

- `ENode`: Hash-consed expression nodes
- `EGraph`: Main e-graph with term insertion/unification

### Query System

- `Query` (`src/query.h`): Conjunctive queries with constraints
- `Compiler` (`src/compiler.h`): Expression → query conversion
  - **Invariant**: Child expressions always receive lower variable IDs than their parents
  - **Format**: Constraints are built as `op(arg1, arg2, ..., argN; id)` with ID in LAST position
- `Engine` (`src/engine.h`): Query execution engine

## File Structure

```
src/: Core components (15 headers, 14 implementations)
├── relations/abstract_relation.h
├── indices/abstract_index.h
├── sets/abstract_set.h
unittests/: 6 test files
systemtests/: 3 test files
```

## Usage Examples

### Theory Frontend

```cpp
Theory theory;

theory.add_operator("1", 0);
theory.add_operator("*", 2);

EGraph egraph(theory);
```

### Term Insertion

```cpp
// ...
auto expr = Expr::make_operator(mul, {x_expr, y_expr});
id_t expr_id = egraph.add_expr(expr);
```

## Bug Fixes

1. **Empty matches/heads handling**: Added validation in `EGraph::apply_matches()` to gracefully handle empty match vectors or zero head_size, preventing division by zero and invalid memory access.

2. **Index recreation after clearing**: Fixed the saturation loop to properly recreate indices after `clear_indices()` is called. The EGraph now stores required indices information and recreates them between iterations, ensuring indices are available for all saturation iterations.

3. **Constraint variable ordering consistency**: Fixed critical ordering inconsistency where e-class IDs were incorrectly placed at the beginning of constraint variables instead of at the end.

   **Changes**:
   - `src/compiler.cpp`: Modified to assign variable IDs AFTER recursing to children, ensuring children always have lower IDs than parents. Constraint variables are now built as `(arg1, ..., argN, id)` with ID in LAST position.
   - `src/egraph.cpp`: Fixed match processing to read LHS root ID from `match[head_size - 1]` (last position) instead of `match[0]`.
   - `unittests/test_compiler.cpp`: Updated all tests to reflect correct variable ordering.

   **Invariants enforced**:
   - Relations/constraints use format: `op(arg1, arg2, ..., argN; eclass_id)` with ID LAST
   - For any expression, child variable IDs < parent variable ID
   - Database storage, query constraints, and match processing all use consistent ordering
