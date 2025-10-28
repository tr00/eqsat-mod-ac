#include "database.h"
#include "symbol_table.h"
#include <catch2/catch_test_macros.hpp>

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
        db.populate_index(add_sym, 0); // Identity permutation [0,1,2]
        db.populate_index(add_sym, 2); // Permutation [1,0,2]
        db.populate_index(mul_sym, 0); // Identity permutation [0,1]
        db.populate_index(mul_sym, 1); // Permutation [1,0]

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

        // Create and populate indices with different permutations
        db.populate_index(add_sym, 0); // Permutation [0,1,2]: (100,200,300), (400,500,600), (700,800,900)
        db.populate_index(add_sym, 4); // Permutation [2,0,1]: (300,100,200), (600,400,500), (900,700,800)
        db.populate_index(add_sym, 5); // Permutation [2,1,0]: (300,200,100), (600,500,400), (900,800,700)

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

        // Create and populate indices for both relations
        db.populate_index(add_sym, 0); // Identity for add
        db.populate_index(add_sym, 1); // Swap for add
        db.populate_index(mul_sym, 0); // Identity for mul
        db.populate_index(mul_sym, 2); // [1,0,2] for mul

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

        // Create and populate several indices
        db.populate_index(add_sym, 0);
        db.populate_index(add_sym, 1);
        db.populate_index(mul_sym, 0);
        db.populate_index(mul_sym, 1);

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
        // Create relation and add index
        db.create_relation(add_sym, 2);

        // Build index on empty relation
        db.populate_index(add_sym, 0); // Should not crash

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

        // Create and populate index
        db.populate_index(add_sym, 0);
        REQUIRE(db.has_index(add_sym, 0));

        // Replace with same key
        db.populate_index(add_sym, 0);
        REQUIRE(db.has_index(add_sym, 0));
    }
}

TEST_CASE("Database AC relation operations", "[database][ac]")
{
    SymbolTable symbol_table;
    Database db;

    // Create AC operator symbol
    Symbol ac_mul_sym = symbol_table.intern("ac_mul");
    Symbol ac_add_sym = symbol_table.intern("ac_add");

    SECTION("AC relation creation and tuple insertion")
    {
        // Create AC relation
        db.create_relation_ac(ac_mul_sym);
        REQUIRE(db.has_relation(ac_mul_sym));

        // Add tuples to AC relation
        // AC relations store multisets, so order shouldn't matter
        db.add_tuple(ac_mul_sym, {1, 2, 3}); // ac_mul(1, 2, 3)
        db.add_tuple(ac_mul_sym, {3, 2, 1}); // Should be same as above (commutative)
        db.add_tuple(ac_mul_sym, {4, 5, 6}); // Different tuple
    }

    SECTION("AC relation index normalization - permutation always becomes 0")
    {
        db.create_relation_ac(ac_mul_sym);
        db.add_tuple(ac_mul_sym, {10, 20, 30});

        // Create and populate index with various permutation values
        // All should normalize to permutation 0 for AC relations
        db.populate_index(ac_mul_sym, 5);   // Should normalize to 0
        db.populate_index(ac_mul_sym, 100); // Should replace previous (same key)
        db.populate_index(ac_mul_sym, 0);   // Explicit 0

        // After normalization, only ONE index should exist (at permutation 0)
        REQUIRE(db.has_index(ac_mul_sym, 0));   // Direct check
        REQUIRE(db.has_index(ac_mul_sym, 5));   // Should normalize to 0 and return true
        REQUIRE(db.has_index(ac_mul_sym, 100)); // Should normalize to 0 and return true
        REQUIRE(db.has_index(ac_mul_sym, 42));  // Any permutation should find the index

        // Should be able to get index with any permutation value
        auto idx_0 = db.get_index(ac_mul_sym, 0);
        auto idx_5 = db.get_index(ac_mul_sym, 5);     // Should return same index
        auto idx_100 = db.get_index(ac_mul_sym, 100); // Should return same index
    }

    SECTION("AC relation index building")
    {
        db.create_relation_ac(ac_mul_sym);

        // Add some tuples
        db.add_tuple(ac_mul_sym, {1, 2, 3});
        db.add_tuple(ac_mul_sym, {4, 5, 6});
        db.add_tuple(ac_mul_sym, {7, 8, 9});

        // Create and populate index (any permutation should work)
        db.populate_index(ac_mul_sym, 42); // Should not crash

        // Index should still exist after building
        REQUIRE(db.has_index(ac_mul_sym, 0));
        REQUIRE(db.has_index(ac_mul_sym, 42));

        // Should be able to retrieve built index
        auto idx = db.get_index(ac_mul_sym, 0);
    }

    SECTION("Mixed AC and regular relations")
    {
        Symbol regular_add = symbol_table.intern("regular_add");

        // Create both types of relations
        db.create_relation_ac(ac_mul_sym);  // AC relation
        db.create_relation(regular_add, 3); // Regular relation

        // Add tuples to both
        db.add_tuple(ac_mul_sym, {1, 2, 3});
        db.add_tuple(regular_add, {10, 20, 30});

        // Create and populate indices
        db.populate_index(ac_mul_sym, 5);  // AC - will normalize to 0
        db.populate_index(regular_add, 0); // Regular - stays 0
        db.populate_index(regular_add, 2); // Regular - stays 2

        // AC relation: any permutation maps to 0
        REQUIRE(db.has_index(ac_mul_sym, 0));
        REQUIRE(db.has_index(ac_mul_sym, 5));
        REQUIRE(db.has_index(ac_mul_sym, 999));

        // Regular relation: specific permutations exist
        REQUIRE(db.has_index(regular_add, 0));
        REQUIRE(db.has_index(regular_add, 2));
        REQUIRE_FALSE(db.has_index(regular_add, 5));   // This permutation wasn't created
        REQUIRE_FALSE(db.has_index(regular_add, 999)); // This permutation wasn't created
    }

    SECTION("Multiple AC relations")
    {
        db.create_relation_ac(ac_mul_sym);
        db.create_relation_ac(ac_add_sym);

        db.add_tuple(ac_mul_sym, {1, 2});
        db.add_tuple(ac_add_sym, {3, 4});

        // Create and populate indices for both AC relations
        db.populate_index(ac_mul_sym, 10);
        db.populate_index(ac_add_sym, 20);

        // Both should be accessible with any permutation
        REQUIRE(db.has_index(ac_mul_sym, 0));
        REQUIRE(db.has_index(ac_mul_sym, 10));
        REQUIRE(db.has_index(ac_add_sym, 0));
        REQUIRE(db.has_index(ac_add_sym, 20));
    }

    SECTION("AC relation index clearing")
    {
        db.create_relation_ac(ac_mul_sym);
        db.add_tuple(ac_mul_sym, {1, 2, 3});

        db.populate_index(ac_mul_sym, 5);

        REQUIRE(db.has_index(ac_mul_sym, 0));
        REQUIRE(db.has_index(ac_mul_sym, 5));

        // Clear indices
        db.clear_indices();

        // All permutation checks should now return false
        REQUIRE_FALSE(db.has_index(ac_mul_sym, 0));
        REQUIRE_FALSE(db.has_index(ac_mul_sym, 5));
        REQUIRE_FALSE(db.has_index(ac_mul_sym, 100));

        // Relation should still exist
        REQUIRE(db.has_relation(ac_mul_sym));
    }
}
