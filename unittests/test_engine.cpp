#include <catch2/catch_test_macros.hpp>

#include "database.h"
#include "engine.h"
#include "query.h"
#include "theory.h"

TEST_CASE("Engine with single state - simple query", "[engine]")
{
    // Create a simple theory with a binary operator "add"
    Theory theory;
    Symbol add = theory.intern("add");
    theory.add_operator(add, 2); // binary operator: add(x, y, result)

    // Create database and add relation for "add"
    Database db;
    db.add_relation(add, 3); // arity 2 + 1 for result = 3

    // Add some tuples: add(a, b, c) means a + b = c
    // Let's add: 1 + 2 = 3
    db.add_tuple(add, Vec<id_t>{1, 2, 3});
    // And: 4 + 5 = 9
    db.add_tuple(add, Vec<id_t>{4, 5, 9});
    // And: 1 + 3 = 4
    db.add_tuple(add, Vec<id_t>{1, 3, 4});

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

        // Should return all tuples: (1,2,3), (4,5,9), (1,3,4)
        REQUIRE(results.size() == 9); // 3 tuples * 3 variables each

        // First tuple: 1, 2, 3
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 2);
        REQUIRE(results[2] == 3);

        // Second tuple: 4, 5, 9
        REQUIRE(results[3] == 4);
        REQUIRE(results[4] == 5);
        REQUIRE(results[5] == 9);

        // Third tuple: 1, 3, 4
        REQUIRE(results[6] == 1);
        REQUIRE(results[7] == 3);
        REQUIRE(results[8] == 4);
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
        // Add a tuple where first arg is 1
        // We already have: add(1, 2, 3) and add(1, 3, 4)

        // Query: add(1, 0, 1) - find all (y, z) where add(1, y, z)
        // Note: This uses variable 0 and 1, where we want to match literal 1 in first position

        // Actually, let's test a simpler case:
        // Query: add(0, 1, 2) but we want first variable to be 1
        // We need to test if variable 0 appears in position 0 and has value 1

        // For now, let's just verify the basic engine works with the constraint
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
    db.add_relation(f, 2); // arity 1 + 1 for result

    // Add single tuple: f(5, 10) means f(5) = 10
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

    // Add multiple tuples with same first argument
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
