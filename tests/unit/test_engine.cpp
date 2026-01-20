#include <catch2/catch_test_macros.hpp>

#include "database.h"
#include "egraph.h"
#include "engine.h"
#include "handle.h"
#include "query.h"
#include "query_builder.h"
#include "theory.h"

using namespace eqsat;
using namespace eqsat::test;

TEST_CASE("Engine with single state - simple query", "[engine]")
{
    // Create a simple theory with a binary operator "add"
    Theory theory;

    Symbol add = theory.add_operator("add", 2);

    // Create database and add relation for "add"
    Database db;
    db.create_relation(add, 3); // add(arg1, arg2; eclass_id)

    // Add some tuples: add(a, b; id) means add(a,b) is in e-class id
    // add(1, 2; 10) - add(1,2) is in e-class 10
    db.add_tuple(add, Vec<id_t>{1, 2, 10});
    // add(4, 5; 11) - add(4,5) is in e-class 11
    db.add_tuple(add, Vec<id_t>{4, 5, 11});
    // add(1, 3; 12) - add(1,3) is in e-class 12
    db.add_tuple(add, Vec<id_t>{1, 3, 12});

    // Build index for the relation
    db.populate_index(add, 0); // identity permutation

    EGraph egraph(theory);

    SECTION("Query for all results - single constraint")
    {
        // find all (x, y, z) where add(x, y, z)
        Query query = QueryBuilder(theory, "Q").with_constraint(add, {0, 1, 2}).with_head_vars({0, 1, 2}).build();

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Should return all tuples: (1,2,10), (4,5,11), (1,3,12)
        REQUIRE(results.size() == 9); // 3 tuples * 3 variables each

        // Verify we got all the tuples (order may vary)
        bool found_1_2_10 = false;
        bool found_4_5_11 = false;
        bool found_1_3_12 = false;

        for (size_t i = 0; i < results.size(); i += 3)
        {
            if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 10)
                found_1_2_10 = true;
            if (results[i] == 4 && results[i + 1] == 5 && results[i + 2] == 11)
                found_4_5_11 = true;
            if (results[i] == 1 && results[i + 1] == 3 && results[i + 2] == 12)
                found_1_3_12 = true;
        }

        REQUIRE(found_1_2_10);
        REQUIRE(found_4_5_11);
        REQUIRE(found_1_3_12);
    }

    // NOT ALLOWED YET
    // SECTION("Query with shared variable - single state")
    // {
    //     // Query: add(0, 0, 1) - find all (x, y) where add(x, x, y)
    //     // This should match nothing in our data since we don't have add(a, a, b)
    //     Query query(theory.intern("test_query2"));
    //     query.add_constraint(add, Vec<var_t>{0, 0, 1});
    //     query.add_head_var(0); // x
    //     query.add_head_var(1); // y

    //     Engine engine(db, Handle(egraph));
    //     engine.prepare(query);
    //     Vec<id_t> results = engine.execute();

    //     // No matches expected
    //     REQUIRE(results.size() == 0);
    // }
}

TEST_CASE("Engine with single state - empty database", "[engine]")
{
    Theory theory;
    Symbol mul = theory.intern("mul");
    theory.add_operator(mul, 2);

    Database db;
    db.create_relation(mul, 3);

    // Build index but don't add any tuples
    db.populate_index(mul, 0);

    EGraph egraph(theory);

    Query query = QueryBuilder(theory, "Q").with_constraint(mul, {0, 1, 2}).with_head_vars({0, 1, 2}).build();

    Engine engine(db, Handle(egraph));
    Vec<id_t> results;
    engine.execute(results, query);

    REQUIRE(results.size() == 0);
}

TEST_CASE("Engine with single state - single tuple", "[engine]")
{
    Theory theory;
    Symbol f = theory.intern("f");
    theory.add_operator(f, 1); // unary operator

    Database db;
    db.create_relation(f, 2); // arity 1 + 1 for e-class id

    // Add single tuple: f(5; 10) means f(5) is in e-class 10
    db.add_tuple(f, Vec<id_t>{5, 10});

    db.populate_index(f, 0);

    EGraph egraph(theory);

    Query query(theory.intern("single_tuple_query"));
    query.add_constraint(Constraint(f, Vec<var_t>{0, 1}));
    query.add_head_var(0);
    query.add_head_var(1);

    Engine engine(db, Handle(egraph));
    Vec<id_t> results;
    engine.execute(results, query);

    REQUIRE(results.size() == 2);
    REQUIRE(results[0] == 5);
    REQUIRE(results[1] == 10);
}

TEST_CASE("Engine state intersection", "[engine]")
{
    Theory theory;
    Symbol g = theory.intern("g");
    theory.add_operator(g, 2);

    Database db;
    db.create_relation(g, 3);

    // Add multiple tuples: g(a, b; id) means g(a,b) is in e-class id
    db.add_tuple(g, Vec<id_t>{1, 2, 3});
    db.add_tuple(g, Vec<id_t>{1, 4, 5});
    db.add_tuple(g, Vec<id_t>{2, 3, 6});

    db.populate_index(g, 0);

    EGraph egraph(theory);

    Query query(theory.intern("intersection_query"));
    query.add_constraint(Constraint(g, Vec<var_t>{0, 1, 2}));
    query.add_head_var(0);
    query.add_head_var(1);
    query.add_head_var(2);

    Engine engine(db, Handle(egraph));
    Vec<id_t> results;
    engine.execute(results, query);

    // Should find all 3 tuples
    REQUIRE(results.size() == 9);

    // Verify we got all the tuples
    bool found_1_2_3 = false;
    bool found_1_4_5 = false;
    bool found_2_3_6 = false;

    for (size_t i = 0; i < results.size(); i += 3)
    {
        if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 3)
            found_1_2_3 = true;
        if (results[i] == 1 && results[i + 1] == 4 && results[i + 2] == 5)
            found_1_4_5 = true;
        if (results[i] == 2 && results[i + 1] == 3 && results[i + 2] == 6)
            found_2_3_6 = true;
    }

    REQUIRE(found_1_2_3);
    REQUIRE(found_1_4_5);
    REQUIRE(found_2_3_6);
}

TEST_CASE("Engine multi-state join - two constraints", "[engine][multi-state]")
{
    Theory theory;
    Symbol add = theory.add_operator("add", 2);
    Symbol mul = theory.add_operator("mul", 2);

    Database db;
    db.create_relation(add, 3); // add(a, b; id)
    db.create_relation(mul, 3); // mul(a, b; id)

    // Setup: add(1, 2; 10), add(3, 4; 11)
    db.add_tuple(add, Vec<id_t>{1, 2, 10});
    db.add_tuple(add, Vec<id_t>{3, 4, 11});

    // Setup: mul(10, 5; 20), mul(11, 6; 21), mul(10, 7; 22)
    db.add_tuple(mul, Vec<id_t>{10, 5, 20});
    db.add_tuple(mul, Vec<id_t>{11, 6, 21});
    db.add_tuple(mul, Vec<id_t>{10, 7, 22});

    // Create indices for identity permutation
    db.populate_index(add, 0);
    db.populate_index(mul, 0);

    EGraph egraph(theory);

    SECTION("Join on shared variable")
    {
        // Query: add(x, y; z), mul(z, w; r)
        // Find all (x, y, z, w, r) where add(x,y) produces z and mul(z,w) produces r
        Query query(theory.intern("join_query"));
        query.add_constraint(Constraint(add, Vec<var_t>{0, 1, 2})); // x=0, y=1, z=2
        query.add_constraint(Constraint(mul, Vec<var_t>{2, 3, 4})); // z=2, w=3, r=4
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Expected matches:
        // (1, 2, 10, 5, 20) - add(1,2;10), mul(10,5;20)
        // (1, 2, 10, 7, 22) - add(1,2;10), mul(10,7;22)
        // (3, 4, 11, 6, 21) - add(3,4;11), mul(11,6;21)
        REQUIRE(results.size() == 15); // 3 matches * 5 variables

        bool found_1_2_10_5_20 = false;
        bool found_1_2_10_7_22 = false;
        bool found_3_4_11_6_21 = false;

        for (size_t i = 0; i < results.size(); i += 5)
        {
            if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 10 && results[i + 3] == 5 &&
                results[i + 4] == 20)
                found_1_2_10_5_20 = true;
            if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 10 && results[i + 3] == 7 &&
                results[i + 4] == 22)
                found_1_2_10_7_22 = true;
            if (results[i] == 3 && results[i + 1] == 4 && results[i + 2] == 11 && results[i + 3] == 6 &&
                results[i + 4] == 21)
                found_3_4_11_6_21 = true;
        }

        REQUIRE(found_1_2_10_5_20);
        REQUIRE(found_1_2_10_7_22);
        REQUIRE(found_3_4_11_6_21);
    }
}

TEST_CASE("Engine multi-state join - three constraints", "[engine][multi-state]")
{
    Theory theory;
    Symbol f = theory.add_operator("f", 1);
    Symbol g = theory.add_operator("g", 2);
    Symbol h = theory.add_operator("h", 1);

    Database db;
    db.create_relation(f, 2); // f(a; id)
    db.create_relation(g, 3); // g(a, b; id)
    db.create_relation(h, 2); // h(a; id)

    // f: f(1; 10), f(2; 11), f(3; 12)
    db.add_tuple(f, Vec<id_t>{1, 10});
    db.add_tuple(f, Vec<id_t>{2, 11});
    db.add_tuple(f, Vec<id_t>{3, 12});

    // g: g(10, 20; 30), g(11, 21; 31), g(12, 22; 32), g(10, 23; 33)
    db.add_tuple(g, Vec<id_t>{10, 20, 30});
    db.add_tuple(g, Vec<id_t>{11, 21, 31});
    db.add_tuple(g, Vec<id_t>{12, 22, 32});
    db.add_tuple(g, Vec<id_t>{10, 23, 33});

    // h: h(30; 40), h(31; 41), h(33; 43)
    db.add_tuple(h, Vec<id_t>{30, 40});
    db.add_tuple(h, Vec<id_t>{31, 41});
    db.add_tuple(h, Vec<id_t>{33, 43});

    db.populate_index(f, 0);
    db.populate_index(g, 0);
    db.populate_index(h, 0);

    EGraph egraph(theory);

    SECTION("Three-way join: f(x; y), g(y, z; w), h(w; r)")
    {
        // Query chains: x -> y (via f) -> w (via g with z) -> r (via h)
        Query query(theory.intern("three_join"));
        query.add_constraint(Constraint(f, Vec<var_t>{0, 1}));    // x=0, y=1
        query.add_constraint(Constraint(g, Vec<var_t>{1, 2, 3})); // y=1, z=2, w=3
        query.add_constraint(Constraint(h, Vec<var_t>{3, 4}));    // w=3, r=4
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Expected matches:
        // (1, 10, 20, 30, 40) - f(1;10), g(10,20;30), h(30;40)
        // (2, 11, 21, 31, 41) - f(2;11), g(11,21;31), h(31;41)
        // (1, 10, 23, 33, 43) - f(1;10), g(10,23;33), h(33;43)
        REQUIRE(results.size() == 15); // 3 matches * 5 variables

        bool found_1_10_20_30_40 = false;
        bool found_2_11_21_31_41 = false;
        bool found_1_10_23_33_43 = false;

        for (size_t i = 0; i < results.size(); i += 5)
        {
            if (results[i] == 1 && results[i + 1] == 10 && results[i + 2] == 20 && results[i + 3] == 30 &&
                results[i + 4] == 40)
                found_1_10_20_30_40 = true;
            if (results[i] == 2 && results[i + 1] == 11 && results[i + 2] == 21 && results[i + 3] == 31 &&
                results[i + 4] == 41)
                found_2_11_21_31_41 = true;
            if (results[i] == 1 && results[i + 1] == 10 && results[i + 2] == 23 && results[i + 3] == 33 &&
                results[i + 4] == 43)
                found_1_10_23_33_43 = true;
        }

        REQUIRE(found_1_10_20_30_40);
        REQUIRE(found_2_11_21_31_41);
        REQUIRE(found_1_10_23_33_43);
    }
}

TEST_CASE("Engine multi-state - variable appears in multiple constraints", "[engine][multi-state][triangle]")
{
    Theory theory;
    Symbol p = theory.add_operator("p", 2);

    Database db;
    db.create_relation(p, 2);

    db.add_tuple(p, Vec<id_t>{1, 2});
    db.add_tuple(p, Vec<id_t>{2, 3});
    db.add_tuple(p, Vec<id_t>{3, 4});
    db.add_tuple(p, Vec<id_t>{3, 1});

    db.populate_index(p, 0);
    db.populate_index(p, 1);

    EGraph egraph(theory);

    // TRIANGLE QUERY: (x, y) (y, z) (z, x)

    Query query(theory.intern("triangle"));
    query.add_constraint(Constraint(p, Vec<var_t>{0, 1}));
    query.add_constraint(Constraint(p, Vec<var_t>{1, 2}));
    query.add_constraint(Constraint(p, Vec<var_t>{2, 0}));
    query.add_head_var(0);
    query.add_head_var(1);
    query.add_head_var(2);

    Engine engine(db, Handle(egraph));
    Vec<id_t> results;
    engine.execute(results, query);

    REQUIRE(results.size() == 9);

    bool found_1_2_3 = false;
    bool found_2_3_1 = false;
    bool found_3_1_2 = false;

    for (size_t i = 0; i < results.size(); i += 3)
    {
        if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 3)
            found_1_2_3 = true;

        if (results[i] == 2 && results[i + 1] == 3 && results[i + 2] == 1)
            found_2_3_1 = true;

        if (results[i] == 3 && results[i + 1] == 1 && results[i + 2] == 2)
            found_3_1_2 = true;
    }

    REQUIRE(found_1_2_3);
    REQUIRE(found_2_3_1);
    REQUIRE(found_3_1_2);
}

TEST_CASE("Engine multi-state - empty intersection with backtracking", "[engine][multi-state]")
{
    Theory theory;
    Symbol a = theory.add_operator("a", 2);
    Symbol b = theory.add_operator("b", 2);

    Database db;
    db.create_relation(a, 3);
    db.create_relation(b, 3);

    // a: a(1, 2; 10), a(3, 4; 11)
    db.add_tuple(a, Vec<id_t>{1, 2, 10});
    db.add_tuple(a, Vec<id_t>{3, 4, 11});

    // b: b(99, 5; 20) - no matching IDs with 'a' outputs
    db.add_tuple(b, Vec<id_t>{99, 5, 20});

    db.populate_index(a, 0);
    db.populate_index(b, 0);

    EGraph egraph(theory);

    SECTION("No matches due to disjoint e-class IDs")
    {
        // Query: a(x, y; z), b(z, w; r)
        // No z from 'a' matches first arg of 'b'
        Query query(theory.intern("no_match"));
        query.add_constraint(Constraint(a, Vec<var_t>{0, 1, 2}));
        query.add_constraint(Constraint(b, Vec<var_t>{2, 3, 4}));
        query.add_head_var(0);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        REQUIRE(results.size() == 0);
    }
}

TEST_CASE("Engine multi-state - shared variable at different positions", "[engine][multi-state]")
{
    Theory theory;
    Symbol op = theory.add_operator("op", 2);

    Database db;
    db.create_relation(op, 3);

    // op(1, 5; 10), op(2, 1; 11), op(3, 2; 12)
    db.add_tuple(op, Vec<id_t>{1, 5, 10});
    db.add_tuple(op, Vec<id_t>{2, 1, 11});
    db.add_tuple(op, Vec<id_t>{3, 2, 12});

    db.populate_index(op, 0);
    db.populate_index(op, 2);

    EGraph egraph(theory);

    SECTION("Variable x appears as first arg in first constraint, second arg in second")
    {
        // Query: op(x, y; z1), op(w, x; z2)
        // x connects position 0 of first to position 1 of second
        Query query(theory.intern("cross_pos"));
        query.add_constraint(Constraint(op, Vec<var_t>{0, 1, 2})); // x=0, y=1, z1=2
        query.add_constraint(Constraint(op, Vec<var_t>{3, 0, 4})); // w=3, x=0, z2=4
        query.add_head_var(0);                                     // x
        query.add_head_var(1);                                     // y
        query.add_head_var(3);                                     // w

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Expected matches:
        // x=1: op(1,5;10), op(2,1;11) -> (1, 5, 2)
        // x=2: op(2,1;11), op(3,2;12) -> (2, 1, 3)
        REQUIRE(results.size() == 6); // 2 matches * 3 variables

        bool found_1_5_2 = false;
        bool found_2_1_3 = false;

        for (size_t i = 0; i < results.size(); i += 3)
        {
            if (results[i] == 1 && results[i + 1] == 5 && results[i + 2] == 2)
                found_1_5_2 = true;
            if (results[i] == 2 && results[i + 1] == 1 && results[i + 2] == 3)
                found_2_1_3 = true;
        }

        REQUIRE(found_1_5_2);
        REQUIRE(found_2_1_3);
    }
}

TEST_CASE("Engine with non-identity permutations", "[engine][multi-state][permutations]")
{
    Theory theory;
    Symbol add = theory.add_operator("add", 2);
    Symbol mul = theory.add_operator("mul", 2);

    Database db;
    db.create_relation(add, 3); // add(a, b; id)
    db.create_relation(mul, 3); // mul(a, b; id)

    // add: add(1, 2; 100), add(3, 4; 101), add(5, 6; 102)
    db.add_tuple(add, Vec<id_t>{1, 2, 100});
    db.add_tuple(add, Vec<id_t>{3, 4, 101});
    db.add_tuple(add, Vec<id_t>{5, 6, 102});

    // mul: mul(100, 10; 200), mul(101, 20; 201), mul(102, 10; 202), mul(100, 30; 203)
    db.add_tuple(mul, Vec<id_t>{100, 10, 200});
    db.add_tuple(mul, Vec<id_t>{101, 20, 201});
    db.add_tuple(mul, Vec<id_t>{102, 10, 202});
    db.add_tuple(mul, Vec<id_t>{100, 30, 203});

    // Create multiple indices with different permutations
    db.populate_index(add, 0); // [0, 1, 2]
    db.populate_index(add, 2); // [1, 0, 2]

    db.populate_index(mul, 0); // [0, 1, 2]
    db.populate_index(mul, 2); // [1, 0, 2]
    db.populate_index(mul, 4); // [2, 0, 1]

    EGraph egraph(theory);

    SECTION("Query using identity permutation on 'add'")
    {
        // Query: add(x, y; z), mul(z, w; r)
        // Both constraints use identity permutation (permutation 0)
        Query query(theory.intern("identity_perm"));
        query.add_constraint(Constraint(add, Vec<var_t>{0, 1, 2})); // Uses permutation 0
        query.add_constraint(Constraint(mul, Vec<var_t>{2, 3, 4})); // Uses permutation 0
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Expected: (1, 2, 100, 10, 200), (1, 2, 100, 30, 203), (3, 4, 101, 20, 201), (5, 6, 102, 10, 202)
        REQUIRE(results.size() == 20); // 4 matches * 5 variables

        bool found_1_2_100_10_200 = false;
        bool found_1_2_100_30_203 = false;
        bool found_3_4_101_20_201 = false;
        bool found_5_6_102_10_202 = false;

        for (size_t i = 0; i < results.size(); i += 5)
        {
            if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 100 && results[i + 3] == 10 &&
                results[i + 4] == 200)
                found_1_2_100_10_200 = true;
            if (results[i] == 1 && results[i + 1] == 2 && results[i + 2] == 100 && results[i + 3] == 30 &&
                results[i + 4] == 203)
                found_1_2_100_30_203 = true;
            if (results[i] == 3 && results[i + 1] == 4 && results[i + 2] == 101 && results[i + 3] == 20 &&
                results[i + 4] == 201)
                found_3_4_101_20_201 = true;
            if (results[i] == 5 && results[i + 1] == 6 && results[i + 2] == 102 && results[i + 3] == 10 &&
                results[i + 4] == 202)
                found_5_6_102_10_202 = true;
        }

        REQUIRE(found_1_2_100_10_200);
        REQUIRE(found_1_2_100_30_203);
        REQUIRE(found_3_4_101_20_201);
        REQUIRE(found_5_6_102_10_202);
    }

    SECTION("Query using swapped permutation on 'add'")
    {
        // Query: add(y, x; z), mul(z, w; r)
        // First constraint uses permutation 2 (swaps first two args)
        Query query(theory.intern("swapped_perm"));
        query.add_constraint(Constraint(add, Vec<var_t>{1, 0, 2})); // Swapped: y, x instead of x, y
        query.add_constraint(Constraint(mul, Vec<var_t>{2, 3, 4}));
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        // Expected: Same matches but with x and y swapped
        // (2, 1, 100, 10, 200), (2, 1, 100, 30, 203), (4, 3, 101, 20, 201), (6, 5, 102, 10, 202)
        REQUIRE(results.size() == 20); // 4 matches * 5 variables

        bool found_2_1_100_10_200 = false;
        bool found_2_1_100_30_203 = false;
        bool found_4_3_101_20_201 = false;
        bool found_6_5_102_10_202 = false;

        for (size_t i = 0; i < results.size(); i += 5)
        {
            if (results[i] == 2 && results[i + 1] == 1 && results[i + 2] == 100 && results[i + 3] == 10 &&
                results[i + 4] == 200)
                found_2_1_100_10_200 = true;
            if (results[i] == 2 && results[i + 1] == 1 && results[i + 2] == 100 && results[i + 3] == 30 &&
                results[i + 4] == 203)
                found_2_1_100_30_203 = true;
            if (results[i] == 4 && results[i + 1] == 3 && results[i + 2] == 101 && results[i + 3] == 20 &&
                results[i + 4] == 201)
                found_4_3_101_20_201 = true;
            if (results[i] == 6 && results[i + 1] == 5 && results[i + 2] == 102 && results[i + 3] == 10 &&
                results[i + 4] == 202)
                found_6_5_102_10_202 = true;
        }

        REQUIRE(found_2_1_100_10_200);
        REQUIRE(found_2_1_100_30_203);
        REQUIRE(found_4_3_101_20_201);
        REQUIRE(found_6_5_102_10_202);
    }
}

TEST_CASE("Engine", "[engine]")
{
    Theory theory;

    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", 2);

    Database db;

    db.create_relation(inv, 2);
    db.create_relation(mul, 3);

    // Q(r, a) := mul(t, a; r), inv(a; t)

    Query query(theory.intern("inverse"));
    query.add_constraint(Constraint(mul, Vec<id_t>{1, 0, 2}));
    query.add_constraint(Constraint(inv, Vec<id_t>{0, 1}));
    query.add_head_var(2);
    query.add_head_var(0);

    db.populate_index(inv, 0);
    db.populate_index(mul, 2); // [1, 0, 2]

    EGraph egraph(theory);

    SECTION("SUCCESS")
    {
        db.add_tuple(inv, Vec<id_t>{15, 101});
        db.add_tuple(inv, Vec<id_t>{16, 102});
        db.add_tuple(inv, Vec<id_t>{17, 103});

        db.add_tuple(mul, Vec<id_t>{101, 17, 201});
        db.add_tuple(mul, Vec<id_t>{102, 16, 202});

        // Rebuild indices after adding new data
        db.populate_index(inv, 0);
        db.populate_index(mul, 2);

        Engine engine(db, Handle(egraph));
        Vec<id_t> results;
        engine.execute(results, query);

        REQUIRE(results.size() == 2);
        REQUIRE(results[0] == 202);
        REQUIRE(results[1] == 16);
    }
}
