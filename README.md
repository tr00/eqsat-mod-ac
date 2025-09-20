# Equality Saturation Modulo AC

This project implements equality saturation on top of a small custom built relational database engine.
Terms are stored as tuples in relations and pattern matching is reduced to answering conjunctive queries.

We implement a special purpose virtual machine, just for answering these kinds of queries efficiently.
Each query is compiled to a sequence of nested iterations and set intersections.
