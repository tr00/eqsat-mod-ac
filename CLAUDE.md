# Equality Saturation Modulo AC

C++17 library implementing e-graphs (equality saturation) with custom database for conjunctive queries.

## Build System

- **Build**: `just` (creates `libeqsat.a`)
- **Test**: `just {test/unittests/systemtes}`
- **Format**: `just format`
- **Full**: `just all`

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

### Database (`src/database.h`, `src/relations/`, `src/indices/`)

- `Database`: Named relation container
- `AbstractRelation`: Base relation interface
  - `RowStore`: Row-oriented relation storage
- `AbstractIndex`: Type-erased index wrapper
  - `TrieIndex`: Trie-based indexing for relations

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
- `PatternCompiler` (`src/pattern_compiler.h`): Expression → query conversion
- `QueryCompiler` (`src/query_compiler.h`): Query compilation
- `Engine` (`src/engine.h`): Query execution engine

## File Structure

```
src/: Core components (16 headers, 15 implementations)
├── relations/abstract_relation.h
├── indices/abstract_index.h
├── sets/abstract_set.h
unittests/: 6 test files
systemtests/: 3 test files
```

## Usage Examples

### Theory Frontend

```cpp
SymbolTable symbols;
symbol_t one = symbols.intern("1");
symbol_t mul = symbols.intern("*");
Theory theory;
theory.add_operator(one, 0);
theory.add_operator(mul, 2);
EGraph egraph(theory);
```

### Term Insertion

```cpp
EGraph egraph;
auto expr = Expression::make_operator(mul, {x_expr, y_expr});
id_t expr_id = egraph.add_expr(expr);
```

## #Pattern Compilation

```cpp PatternCompiler compiler;
Query compiled = compiler.compile_pattern(pattern_expr);
```
