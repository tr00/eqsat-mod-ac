# Equality Saturation Modulo AC

This project implements equality saturation on top of a small custom built relational database engine.
Terms are stored as tuples in relations and pattern matching is reduced to answering conjunctive queries.
Please note that this project is a research prototype and not a fully fledged datalog engine.

We implement a special purpose virtual machine, just for answering these kinds of queries efficiently.
Each query is compiled to a sequence of nested iterations and set intersections.

### Example

```cpp
Theory theory;

auto one = theory.add_operator("1", 0);
auto add = theory.add_operator("+", 2);
auto mul = theory.add_operator("*", 2);

theory.add_rewrite_rule("identity", "(* (1) ?x)", "?x");

Egraph egraph(theory);

egraph.saturate(/* iterations: */ 10);
```
