# EqSat-Mod-AC: E-Graph with Custom Database for Conjunctive Queries

## Project Overview
This C++17 library implements an e-graph (equality saturation) built on top of a custom database for answering conjunctive queries.
The library performs algebraic reasoning by pattern matching and rewriting.

## Core Architecture

### Build System
- **CMake** (C++17): `cmake --build build` (builds static library `libeqsat.a`)
- **Unit Testing**: `cmake --build build && ./build/unittests` or `make rununit`
- **System Testing**: `cmake --build build && ./build/systemtests` or `make runsystem`
- **All Tests**: `make runtest` (runs both unit and system tests)
- **Dependencies**: Catch2 for testing (auto-fetched)
- **Library Usage**: Link against `eqsat` target and include `src/` directory

### Key Data Structures

#### Basic Types (`src/id.h`)
- `id_t`: 32-bit unsigned integer for node identifiers
- `symbol_t`: 32-bit unsigned integer for symbol identifiers
- `var_t`: 32-bit unsigned integer for query variables

#### Symbol Management (`src/symbol_table.h`)
- `SymbolTable`: Bidirectional string ↔ symbol_t mapping
- Thread-safe symbol interning for efficient string handling

#### Theory & Expressions (`src/theory.h`)
- `Expression`: AST nodes for algebraic expressions (operators/variables)
- `Signature`: Operator definitions with arity constraints
- `RewriteRule`: Left-hand-side → right-hand-side transformations
- `Theory`: Collection of operators and rewrite rules

### Database Layer

#### Relations (`src/database.h`)
- `Relation`: Fixed-arity tuple storage with efficient access
- `Database`: Named relation container with tuple insertion/lookup

#### Set Operations (`src/set_interface.h`)
- `SetInterface`: Type-erased wrapper for different set implementations
- Supports intersection, iteration, and type conversion
- Template-based design for performance-critical operations

#### Union-Find (`src/union_find.h`)
- `UnionFind`: Disjoint set data structure for equivalence classes
- Path halving optimization and union-by-smaller-id for efficient operations

### E-Graph Implementation

#### Core E-Graph (`src/egraph.h`)
- `ENode`: Hash-consed expression nodes (operator + children)
- `EGraph`: Main e-graph with term insertion and unification
- Integrates theory, database, and union-find for equality reasoning

#### Query Processing (`src/query.h`)
- `Constraint`: Operator applications over query variables
- `Query`: Conjunctive queries with constraints and head variables
- Supports complex pattern matching and variable binding

#### Pattern Compilation (`src/pattern_compiler.h`)
- `PatternCompiler`: Converts expressions to executable queries
- Handles variable binding and constraint generation
- Supports multiple pattern compilation for batch processing

### Query Engine (`src/engine.h`)
- `Engine`: Main query execution engine (work in progress)
- `State`: Query execution states with variable selections
- Set intersection and iteration for conjunctive query answering

## Key Operations

### Term Insertion
```cpp
EGraph egraph;
auto expr = Expression::make_operator(add_sym, {x_expr, y_expr});
id_t term_id = egraph.insert_term(expr);
```

### Pattern Compilation
```cpp
PatternCompiler compiler;
Query compiled = compiler.compile_pattern(pattern_expr);
```

## File Structure
```
src/
│   id.h
│   symbol_table.{h,cpp}
│   theory.{h,cpp}
│   database.{h,cpp}
│   abstract_set.{h,cpp}
│   sorted_set.{h,cpp}
│   union_find.{h,cpp}
│   egraph.{h,cpp}
│   query.{h,cpp}
│   pattern_compiler.{h,cpp}
│   engine.{h,cpp}
└── trie.{h,cpp}

unittests/
│   test_sorted_set.cpp
│   test_abstract_set.cpp
│   test_pattern_compiler.cpp
└── test_union_find.cpp

systemtests/
└── test_egraph_basic.cpp
```

## Development Notes

### Current Status
- Core data structures implemented and tested
- E-graph foundation complete with union-find integration
- Pattern compilation working for expression → query transformation
- Query engine architecture defined but implementation incomplete

### Next Steps
- Complete query engine implementation in `src/engine.cpp`
- Add trie-based indexing for efficient query execution
- Implement rewrite rule application and saturation
- Add benchmarking and performance optimization
