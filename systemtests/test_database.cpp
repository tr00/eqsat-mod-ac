#include "database.h"
#include "symbol_table.h"
#include <catch2/catch_test_macros.hpp>
#include <sstream>

TEST_CASE("Database basic operations", "[database]")
{
    SymbolTable symbol_table;
    Database db;

    // Create operator symbols
    Symbol add_sym = symbol_table.intern("add");
    Symbol mul_sym = symbol_table.intern("mul");

    SECTION("Create relations and insert tuples")
    {
        // Add binary relations for add and mul operators
        db.create_relation(add_sym, 3); // add(result, left, right)
        db.create_relation(mul_sym, 3); // mul(result, left, right)

        // Insert some tuples into add relation
        db.add_tuple(add_sym, {1, 2, 3}); // add(1, 2, 3) means 2 + 3 = 1
        db.add_tuple(add_sym, {4, 1, 5}); // add(4, 1, 5) means 1 + 5 = 4
        db.add_tuple(add_sym, {6, 2, 4}); // add(6, 2, 4) means 2 + 4 = 6

        // Insert some tuples into mul relation
        db.add_tuple(mul_sym, {8, 2, 4});  // mul(8, 2, 4) means 2 * 4 = 8
        db.add_tuple(mul_sym, {10, 5, 2}); // mul(10, 5, 2) means 5 * 2 = 10

        // Verify relations exist
        REQUIRE(db.has_relation(add_sym));
        REQUIRE(db.has_relation(mul_sym));

        // Note: Direct relation access no longer available
        // Relations are accessible only through indices
    }

    SECTION("Test dump functionality")
    {
        // Create a simple relation
        db.create_relation(add_sym, 2);
        db.add_tuple(add_sym, {10, 20});
        db.add_tuple(add_sym, {30, 40});
        db.add_tuple(add_sym, {50, 60});

        // Note: Direct relation access and dump functionality no longer available
        // Would need to test through indices or other available methods
    }
}

TEST_CASE("Database index operations", "[database]")
{
    SymbolTable symbol_table;
    Database db;

    // Create operator symbols
    Symbol add_sym = symbol_table.intern("add");
    Symbol mul_sym = symbol_table.intern("mul");

    SECTION("Index creation and querying")
    {
        // Create relations first
        db.create_relation(add_sym, 3); // add(result, left, right)
        db.create_relation(mul_sym, 2); // mul(left, right)

        // Add some tuples
        db.add_tuple(add_sym, {1, 2, 3});
        db.add_tuple(add_sym, {4, 5, 6});
        db.add_tuple(mul_sym, {7, 8});
        db.add_tuple(mul_sym, {9, 10});

        // Initially no indices should exist
        REQUIRE_FALSE(db.has_index(add_sym, 0));
        REQUIRE_FALSE(db.has_index(add_sym, 1));
        REQUIRE_FALSE(db.has_index(mul_sym, 0));

        // Create indices with different permutations
        db.create_index(add_sym, 0); // Identity permutation [0,1,2]
        db.create_index(add_sym, 2); // Permutation [1,0,2]
        db.create_index(mul_sym, 0); // Identity permutation [0,1]
        db.create_index(mul_sym, 1); // Permutation [1,0]

        // Verify indices exist
        REQUIRE(db.has_index(add_sym, 0));
        REQUIRE(db.has_index(add_sym, 2));
        REQUIRE(db.has_index(mul_sym, 0));
        REQUIRE(db.has_index(mul_sym, 1));

        // Non-existent indices should return false
        REQUIRE_FALSE(db.has_index(add_sym, 5));
        REQUIRE_FALSE(db.has_index(mul_sym, 3));

        // Get index objects (should succeed)
        auto add_idx_0 = db.get_index(add_sym, 0);
        auto add_idx_2 = db.get_index(add_sym, 2);
        auto mul_idx_0 = db.get_index(mul_sym, 0);
        auto mul_idx_1 = db.get_index(mul_sym, 1);

        // Indices are returned by value, so just verify they were retrieved
        // (if they didn't exist, the assertion in get_index would have failed)
    }

    SECTION("Index building with permutations")
    {
        // Create a relation with known data
        db.create_relation(add_sym, 3);

        // Add tuples in specific order for testing
        db.add_tuple(add_sym, {100, 200, 300}); // Tuple 0
        db.add_tuple(add_sym, {400, 500, 600}); // Tuple 1
        db.add_tuple(add_sym, {700, 800, 900}); // Tuple 2

        // Create indices with different permutations
        db.create_index(add_sym, 0); // Permutation [0,1,2]: (100,200,300), (400,500,600), (700,800,900)
        db.create_index(add_sym, 4); // Permutation [2,0,1]: (300,100,200), (600,400,500), (900,700,800)
        db.create_index(add_sym, 5); // Permutation [2,1,0]: (300,200,100), (600,500,400), (900,800,700)

        // Build all indices
        db.populate_indices();

        // Verify indices still exist after building
        REQUIRE(db.has_index(add_sym, 0));
        REQUIRE(db.has_index(add_sym, 4));
        REQUIRE(db.has_index(add_sym, 5));

        // Get built indices (should succeed)
        auto idx_0 = db.get_index(add_sym, 0);
        auto idx_4 = db.get_index(add_sym, 4);
        auto idx_5 = db.get_index(add_sym, 5);

        // Note: We can't easily test the internal structure of the trie indices
        // without access to their internal methods, but we can verify they were created
        // and the build_indices() function completed without errors
    }

    SECTION("Multiple relations with indices")
    {
        // Create multiple relations
        db.create_relation(add_sym, 2);
        db.create_relation(mul_sym, 3);

        // Add data to both relations
        db.add_tuple(add_sym, {10, 20});
        db.add_tuple(add_sym, {30, 40});
        db.add_tuple(mul_sym, {100, 200, 300});
        db.add_tuple(mul_sym, {400, 500, 600});

        // Create indices for both relations
        db.create_index(add_sym, 0); // Identity for add
        db.create_index(add_sym, 1); // Swap for add
        db.create_index(mul_sym, 0); // Identity for mul
        db.create_index(mul_sym, 2); // [1,0,2] for mul

        // Build all indices
        db.populate_indices();

        // Verify all indices exist
        REQUIRE(db.has_index(add_sym, 0));
        REQUIRE(db.has_index(add_sym, 1));
        REQUIRE(db.has_index(mul_sym, 0));
        REQUIRE(db.has_index(mul_sym, 2));
    }

    SECTION("Index clearing functionality")
    {
        // Set up relations and indices
        db.create_relation(add_sym, 2);
        db.create_relation(mul_sym, 2);

        db.add_tuple(add_sym, {1, 2});
        db.add_tuple(mul_sym, {3, 4});

        // Create several indices
        db.create_index(add_sym, 0);
        db.create_index(add_sym, 1);
        db.create_index(mul_sym, 0);
        db.create_index(mul_sym, 1);

        // Build indices
        db.populate_indices();

        // Verify indices exist
        REQUIRE(db.has_index(add_sym, 0));
        REQUIRE(db.has_index(add_sym, 1));
        REQUIRE(db.has_index(mul_sym, 0));
        REQUIRE(db.has_index(mul_sym, 1));

        // Clear all indices
        db.clear_indices();

        // Verify all indices are gone
        REQUIRE_FALSE(db.has_index(add_sym, 0));
        REQUIRE_FALSE(db.has_index(add_sym, 1));
        REQUIRE_FALSE(db.has_index(mul_sym, 0));
        REQUIRE_FALSE(db.has_index(mul_sym, 1));

        // Verify indices no longer exist after clearing
        REQUIRE_FALSE(db.has_index(add_sym, 0));
        REQUIRE_FALSE(db.has_index(add_sym, 1));
        REQUIRE_FALSE(db.has_index(mul_sym, 0));
        REQUIRE_FALSE(db.has_index(mul_sym, 1));

        // Relations should still exist and be intact
        REQUIRE(db.has_relation(add_sym));
        REQUIRE(db.has_relation(mul_sym));

        // Note: Direct relation access no longer available
        // Would need to test relation integrity through indices or other methods
    }

    SECTION("Edge cases and error conditions")
    {
        // Test building indices without relations
        db.populate_indices(); // Should not crash

        // Create relation and add index
        db.create_relation(add_sym, 2);
        db.create_index(add_sym, 0);

        // Build indices on empty relation
        db.populate_indices(); // Should not crash

        REQUIRE(db.has_index(add_sym, 0));

        // Clear empty indices
        db.clear_indices();
        REQUIRE_FALSE(db.has_index(add_sym, 0));

        // Multiple clears should be safe
        db.clear_indices(); // Should not crash
    }

    SECTION("Index replacement")
    {
        db.create_relation(add_sym, 2);
        db.add_tuple(add_sym, {1, 2});

        // Create index
        db.create_index(add_sym, 0);
        REQUIRE(db.has_index(add_sym, 0));

        // Replace with same key
        db.create_index(add_sym, 0);
        REQUIRE(db.has_index(add_sym, 0));

        // Should still work after building
        db.populate_indices();
        REQUIRE(db.has_index(add_sym, 0));
    }
}
