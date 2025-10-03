#include <catch2/catch_test_macros.hpp>

#include "database.h"
#include "engine.h"
#include "permutation.h"
#include "query.h"
#include "theory.h"

TEST_CASE("Engine with single state - simple query", "[engine]")
{
    // Create a simple theory with a binary operator "add"
    Theory theory;
    Symbol add = theory.add_operator("add", 2);

    // Create database and add relation for "add"
    Database db;
    db.add_relation(add, 3); // add(arg1, arg2; eclass_id)

    // Add some tuples: add(a, b; id) means add(a,b) is in e-class id
    // add(1, 2; 10) - add(1,2) is in e-class 10
    db.add_tuple(add, Vec<id_t>{1, 2, 10});
    // add(4, 5; 11) - add(4,5) is in e-class 11
    db.add_tuple(add, Vec<id_t>{4, 5, 11});
    // add(1, 3; 12) - add(1,3) is in e-class 12
    db.add_tuple(add, Vec<id_t>{1, 3, 12});

    // Build index for the relation
    db.add_index(add, 0); // identity permutation
    db.build_indices();

    SECTION("Query for all results - single constraint")
    {
        // Query: add(0, 1, 2) - find all (x, y, z) where add(x, y, z)
        Query query(theory.intern("test_query"));
        query.add_constraint(add, Vec<var_t>{0, 1, 2});
        query.add_head_var(0); // x
        query.add_head_var(1); // y
        query.add_head_var(2); // z

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        // Should return all tuples: (1,2,10), (4,5,11), (1,3,12)
        REQUIRE(results.size() == 9); // 3 tuples * 3 variables each

        // First tuple: 1, 2, 10
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 2);
        REQUIRE(results[2] == 10);

        // Second tuple: 4, 5, 11
        REQUIRE(results[3] == 4);
        REQUIRE(results[4] == 5);
        REQUIRE(results[5] == 11);

        // Third tuple: 1, 3, 12
        REQUIRE(results[6] == 1);
        REQUIRE(results[7] == 3);
        REQUIRE(results[8] == 12);
    }

    SECTION("Query with shared variable - single state")
    {
        // Query: add(0, 0, 1) - find all (x, y) where add(x, x, y)
        // This should match nothing in our data since we don't have add(a, a, b)
        Query query(theory.intern("test_query2"));
        query.add_constraint(add, Vec<var_t>{0, 0, 1});
        query.add_head_var(0); // x
        query.add_head_var(1); // y

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        // No matches expected
        REQUIRE(results.size() == 0);
    }

    SECTION("Query with constant pattern")
    {
        // We already have: add(1, 2; 10) and add(1, 3; 12)
        // Both have first arg = 1

        // Query: find all (x, y, z) where add(x, y; z)
        Query query(theory.intern("test_query3"));
        query.add_constraint(add, Vec<var_t>{0, 1, 2});
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        REQUIRE(results.size() > 0); // Should find matches
    }
}

TEST_CASE("Engine with single state - empty database", "[engine]")
{
    Theory theory;
    Symbol mul = theory.intern("mul");
    theory.add_operator(mul, 2);

    Database db;
    db.add_relation(mul, 3);

    // Build index but don't add any tuples
    db.add_index(mul, 0);
    db.build_indices();

    Query query(theory.intern("empty_query"));
    query.add_constraint(mul, Vec<var_t>{0, 1, 2});
    query.add_head_var(0);
    query.add_head_var(1);
    query.add_head_var(2);

    Engine engine(db);
    engine.prepare(query);
    Vec<id_t> results = engine.execute();

    REQUIRE(results.size() == 0);
}

TEST_CASE("Engine with single state - single tuple", "[engine]")
{
    Theory theory;
    Symbol f = theory.intern("f");
    theory.add_operator(f, 1); // unary operator

    Database db;
    db.add_relation(f, 2); // arity 1 + 1 for e-class id

    // Add single tuple: f(5; 10) means f(5) is in e-class 10
    db.add_tuple(f, Vec<id_t>{5, 10});

    db.add_index(f, 0);
    db.build_indices();

    Query query(theory.intern("single_tuple_query"));
    query.add_constraint(f, Vec<var_t>{0, 1});
    query.add_head_var(0);
    query.add_head_var(1);

    Engine engine(db);
    engine.prepare(query);
    Vec<id_t> results = engine.execute();

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
    db.add_relation(g, 3);

    // Add multiple tuples: g(a, b; id) means g(a,b) is in e-class id
    db.add_tuple(g, Vec<id_t>{1, 2, 3});
    db.add_tuple(g, Vec<id_t>{1, 4, 5});
    db.add_tuple(g, Vec<id_t>{2, 3, 6});

    db.add_index(g, 0);
    db.build_indices();

    Query query(theory.intern("intersection_query"));
    query.add_constraint(g, Vec<var_t>{0, 1, 2});
    query.add_head_var(0);
    query.add_head_var(1);
    query.add_head_var(2);

    Engine engine(db);
    engine.prepare(query);
    Vec<id_t> results = engine.execute();

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
    db.add_relation(add, 3); // add(a, b; id)
    db.add_relation(mul, 3); // mul(a, b; id)

    // Setup: add(1, 2; 10), add(3, 4; 11)
    db.add_tuple(add, Vec<id_t>{1, 2, 10});
    db.add_tuple(add, Vec<id_t>{3, 4, 11});

    // Setup: mul(10, 5; 20), mul(11, 6; 21), mul(10, 7; 22)
    db.add_tuple(mul, Vec<id_t>{10, 5, 20});
    db.add_tuple(mul, Vec<id_t>{11, 6, 21});
    db.add_tuple(mul, Vec<id_t>{10, 7, 22});

    // Create indices for identity permutation
    db.add_index(add, 0);
    db.add_index(mul, 0);
    db.build_indices();

    SECTION("Join on shared variable")
    {
        // Query: add(x, y; z), mul(z, w; r)
        // Find all (x, y, z, w, r) where add(x,y) produces z and mul(z,w) produces r
        Query query(theory.intern("join_query"));
        query.add_constraint(add, Vec<var_t>{0, 1, 2}); // x=0, y=1, z=2
        query.add_constraint(mul, Vec<var_t>{2, 3, 4}); // z=2, w=3, r=4
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

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
    db.add_relation(f, 2); // f(a; id)
    db.add_relation(g, 3); // g(a, b; id)
    db.add_relation(h, 2); // h(a; id)

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

    db.add_index(f, 0);
    db.add_index(g, 0);
    db.add_index(h, 0);
    db.build_indices();

    SECTION("Three-way join: f(x; y), g(y, z; w), h(w; r)")
    {
        // Query chains: x -> y (via f) -> w (via g with z) -> r (via h)
        Query query(theory.intern("three_join"));
        query.add_constraint(f, Vec<var_t>{0, 1});    // x=0, y=1
        query.add_constraint(g, Vec<var_t>{1, 2, 3}); // y=1, z=2, w=3
        query.add_constraint(h, Vec<var_t>{3, 4});    // w=3, r=4
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

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

TEST_CASE("Engine multi-state - variable appears in multiple constraints", "[engine][multi-state]")
{
    Theory theory;
    Symbol p = theory.add_operator("p", 2);

    Database db;
    db.add_relation(p, 3);

    // p(1, 2; 10), p(2, 3; 11), p(1, 3; 12), p(3, 4; 13)
    db.add_tuple(p, Vec<id_t>{1, 2, 10});
    db.add_tuple(p, Vec<id_t>{2, 3, 11});
    db.add_tuple(p, Vec<id_t>{1, 3, 12});
    db.add_tuple(p, Vec<id_t>{3, 4, 13});

    db.add_index(p, 0);
    db.build_indices();

    SECTION("Triangle query: p(x, y; z1), p(y, z; z2), p(x, z; z3)")
    {
        // Find triangles where x->y, y->z, x->z all exist
        Query query(theory.intern("triangle"));
        query.add_constraint(p, Vec<var_t>{0, 1, 2}); // x=0, y=1, z1=2
        query.add_constraint(p, Vec<var_t>{1, 3, 4}); // y=1, z=3, z2=4
        query.add_constraint(p, Vec<var_t>{0, 3, 5}); // x=0, z=3, z3=5
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(3);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        // Expected: (1, 2, 3) - p(1,2;10), p(2,3;11), p(1,3;12)
        REQUIRE(results.size() == 3);
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 2);
        REQUIRE(results[2] == 3);
    }
}

TEST_CASE("Engine multi-state - empty intersection with backtracking", "[engine][multi-state]")
{
    Theory theory;
    Symbol a = theory.add_operator("a", 2);
    Symbol b = theory.add_operator("b", 2);

    Database db;
    db.add_relation(a, 3);
    db.add_relation(b, 3);

    // a: a(1, 2; 10), a(3, 4; 11)
    db.add_tuple(a, Vec<id_t>{1, 2, 10});
    db.add_tuple(a, Vec<id_t>{3, 4, 11});

    // b: b(99, 5; 20) - no matching IDs with 'a' outputs
    db.add_tuple(b, Vec<id_t>{99, 5, 20});

    db.add_index(a, 0);
    db.add_index(b, 0);
    db.build_indices();

    SECTION("No matches due to disjoint e-class IDs")
    {
        // Query: a(x, y; z), b(z, w; r)
        // No z from 'a' matches first arg of 'b'
        Query query(theory.intern("no_match"));
        query.add_constraint(a, Vec<var_t>{0, 1, 2});
        query.add_constraint(b, Vec<var_t>{2, 3, 4});
        query.add_head_var(0);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        REQUIRE(results.size() == 0);
    }
}

TEST_CASE("Engine multi-state - shared variable at different positions", "[engine][multi-state]")
{
    Theory theory;
    Symbol op = theory.add_operator("op", 2);

    Database db;
    db.add_relation(op, 3);

    // op(1, 5; 10), op(2, 1; 11), op(3, 2; 12)
    db.add_tuple(op, Vec<id_t>{1, 5, 10});
    db.add_tuple(op, Vec<id_t>{2, 1, 11});
    db.add_tuple(op, Vec<id_t>{3, 2, 12});

    db.add_index(op, 0);
    db.build_indices();

    SECTION("Variable x appears as first arg in first constraint, second arg in second")
    {
        // Query: op(x, y; z1), op(w, x; z2)
        // x connects position 0 of first to position 1 of second
        Query query(theory.intern("cross_pos"));
        query.add_constraint(op, Vec<var_t>{0, 1, 2}); // x=0, y=1, z1=2
        query.add_constraint(op, Vec<var_t>{3, 0, 4}); // w=3, x=0, z2=4
        query.add_head_var(0);                         // x
        query.add_head_var(1);                         // y
        query.add_head_var(3);                         // w

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

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
    db.add_relation(add, 3); // add(a, b; id)
    db.add_relation(mul, 3); // mul(a, b; id)

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
    db.add_index(add, 0); // identity: [arg1, arg2, eclass] -> [0, 1, 2]
    db.add_index(add, 2); // permutation 2: [arg2, arg1, eclass] -> [1, 0, 2]
    db.add_index(mul, 0); // identity: [arg1, arg2, eclass] -> [0, 1, 2]
    db.add_index(mul, 4); // permutation 4: [eclass, arg1, arg2] -> [2, 0, 1]

    db.build_indices();

    SECTION("Query using identity permutation on 'add'")
    {
        // Query: add(x, y; z), mul(z, w; r)
        // Both constraints use identity permutation (permutation 0)
        Query query(theory.intern("identity_perm"));
        query.add_constraint(add, Vec<var_t>{0, 1, 2}); // Uses permutation 0
        query.add_constraint(mul, Vec<var_t>{2, 3, 4}); // Uses permutation 0
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

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
        query.add_constraint(add, Vec<var_t>{1, 0, 2}); // Swapped: y, x instead of x, y
        query.add_constraint(mul, Vec<var_t>{2, 3, 4});
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

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

    SECTION("Query using eclass-first permutation on 'mul'")
    {
        // Query: add(x, y; z), mul(w, z; r)
        // Second constraint uses permutation 4: [eclass, arg1, arg2] = [2, 0, 1]
        // So variable order is: [arg2, eclass, arg1]
        Query query(theory.intern("eclass_first_perm"));
        query.add_constraint(add, Vec<var_t>{0, 1, 2}); // x, y, z
        query.add_constraint(mul, Vec<var_t>{3, 2, 4}); // w=arg2, z=eclass, r=arg1
        query.add_head_var(0);
        query.add_head_var(1);
        query.add_head_var(2);
        query.add_head_var(3);
        query.add_head_var(4);

        Engine engine(db);
        engine.prepare(query);
        Vec<id_t> results = engine.execute();

        // For mul tuples: mul(100, 10; 200) in storage
        // With perm 4 [2,0,1]: looks like [200, 100, 10] in index
        // Query mul(w, z; r) = mul(arg2, eclass, arg1) matches to [w=arg2, z=eclass, r=arg1]
        // So: w=10, z=100, r=200 for first tuple

        // Expected matches where add's eclass matches mul's arg1:
        // add(1,2;100) with mul(100,10;200): x=1, y=2, z=100, w=10, r=200
        // add(1,2;100) with mul(100,30;203): x=1, y=2, z=100, w=30, r=203
        // add(3,4;101) with mul(101,20;201): x=3, y=4, z=101, w=20, r=201
        // add(5,6;102) with mul(102,10;202): x=5, y=6, z=102, w=10, r=202
        REQUIRE(results.size() == 20);

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
}

TEST_CASE("Engine", "[engine]")
{
    Theory theory;

    auto inv = theory.add_operator("inv", 1);
    auto mul = theory.add_operator("mul", 2);

    Database db;

    db.add_relation(inv, 2);
    db.add_relation(mul, 3);

    // Q(r, a) := mul(t, a; r), inv(a; t)

    Query query(theory.intern("inverse"));
    query.add_constraint(mul, Vec<id_t>{1, 0, 2});
    query.add_constraint(inv, Vec<id_t>{0, 1});
    query.add_head_var(2);
    query.add_head_var(1);

    db.add_index(inv, 0);
    auto perm = permutation_to_index(Vec<uint32_t>{1, 0, 2});
    db.add_index(mul, perm);

    SECTION("SUCCESS")
    {
        Engine engine(db);

        db.add_tuple(inv, Vec<id_t>{15, 101});
        db.add_tuple(inv, Vec<id_t>{16, 102});
        db.add_tuple(inv, Vec<id_t>{17, 103});

        db.add_tuple(mul, Vec<id_t>{101, 17, 201});
        db.add_tuple(mul, Vec<id_t>{102, 16, 202});

        db.build_indices();

        engine.prepare(query);
        auto results = engine.execute();

        REQUIRE(results.size() == 2);
        REQUIRE(results[0] == 202);
        REQUIRE(results[1] == 202);
    }
}
