# EqSat-Mod-AC: E-Graph with Custom Database for Conjunctive Queries

C++17 library implementing e-graph (equality saturation) with custom database for conjunctive queries.

## Build System
- **Build**: `cmake --build build` (creates `libeqsat.a`)
- **Unit Tests**: `make rununit`
- **System Tests**: `make runsystem`
- **All Tests**: `make runtest`

## Core Components

### Basic Types (`src/id.h`)
- `id_t`, `symbol_t`, `var_t`: 32-bit identifiers

### Symbol Management (`src/symbol_table.h`)
- `SymbolTable`: Thread-safe string ↔ symbol_t mapping

### Theory (`src/theory.h`)
- `Expression`: AST nodes for algebraic expressions
- `Signature`: Operator definitions
- `RewriteRule`: LHS → RHS transformations
- `Theory`: Operators and rewrite rules collection

### Database (`src/database.h`)
- `Relation`: Fixed-arity tuple storage
- `Database`: Named relation container

### Set Operations (`src/sets/`)
- `AbstractSet`: Set wrapper with tagged-union polymorphism
- `SortedSet`: Vector-based sorted set implementation
- `SortedIterSet`: Iterator-based sorted set implementation

### Union-Find (`src/union_find.h`)
- `UnionFind`: Disjoint sets for equivalence classes

### E-Graph (`src/egraph.h`)
- `ENode`: Hash-consed expression nodes
- `EGraph`: Main e-graph with term insertion/unification

### Query System
- `Query` (`src/query.h`): Conjunctive queries with constraints
- `PatternCompiler` (`src/pattern_compiler.h`): Expression → query conversion
- `QueryCompiler` (`src/query_compiler.h`): Query compilation
- `Engine` (`src/engine.h`): Query execution engine

### Indexing (`src/indices/`)
- `AbstractIndex`: Type-erased index wrapper
- `TrieIndex`: Trie-based indexing for relations

### Relations (`src/relations/`)
- `AbstractRelation`: Base relation interface
- `RowStore`: Row-oriented relation storage

### Additional Components
- `Permutation` (`src/permutation.h`): Permutation utilities

## File Structure
```
src/: Core components (16 headers, 15 implementations)
├── sets/: Set implementations (abstract_set, sorted_set, sorted_iter_set)
├── indices/: Indexing structures (abstract_index, trie_index)
├── relations/: Relation storage (abstract_relation, row_store)
unittests/: 6 test files
systemtests/: 3 test files
```

## Usage Examples

### Term Insertion
```cpp
EGraph egraph;
auto expr = Expression::make_operator(add_sym, {x_expr, y_expr});
id_t expr_id = egraph.add_expr(expr);
```

### Pattern Compilation
```cpp
PatternCompiler compiler;
Query compiled = compiler.compile_pattern(pattern_expr);
```
