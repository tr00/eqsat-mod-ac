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

### Pattern Compilation

```cpp
Compiler compiler;
Query compiled = compiler.compile_pattern(pattern_expr);
```
