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

### Set Operations (`src/abstract_set.h`, `src/sorted_set.h`)
- `AbstractSet`: Type-erased set wrapper
- `SortedSet`: Concrete sorted set implementation

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

### Additional Components
- `TrieIndex` (`src/trie_index.h`): Trie-based indexing
- `Permutation` (`src/permutation.h`): Permutation utilities

## File Structure
```
src/: 14 headers, 14 implementations + main.cpp
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