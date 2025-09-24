#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "database.h"
#include "symbol_table.h"

TEST_CASE("Database basic operations", "[database]") {
    SymbolTable symbol_table;
    Database db;
    
    // Create operator symbols
    symbol_t add_sym = symbol_table.intern("add");
    symbol_t mul_sym = symbol_table.intern("mul");
    
    SECTION("Create relations and insert tuples") {
        // Add binary relations for add and mul operators
        db.add_relation(add_sym, 3);  // add(result, left, right)
        db.add_relation(mul_sym, 3);  // mul(result, left, right)
        
        // Insert some tuples into add relation
        db.add_tuple(add_sym, {1, 2, 3});  // add(1, 2, 3) means 2 + 3 = 1
        db.add_tuple(add_sym, {4, 1, 5});  // add(4, 1, 5) means 1 + 5 = 4
        db.add_tuple(add_sym, {6, 2, 4});  // add(6, 2, 4) means 2 + 4 = 6
        
        // Insert some tuples into mul relation
        db.add_tuple(mul_sym, {8, 2, 4});  // mul(8, 2, 4) means 2 * 4 = 8
        db.add_tuple(mul_sym, {10, 5, 2}); // mul(10, 5, 2) means 5 * 2 = 10
        
        // Verify relations exist
        REQUIRE(db.has_relation(add_sym));
        REQUIRE(db.has_relation(mul_sym));
        
        // Test relation properties
        const Relation* add_rel = db.get_relation(add_sym);
        REQUIRE(add_rel != nullptr);
        REQUIRE(add_rel->get_arity() == 3);
        REQUIRE(add_rel->size() == 3);
        REQUIRE(add_rel->get_operator_symbol() == add_sym);
        
        const Relation* mul_rel = db.get_relation(mul_sym);
        REQUIRE(mul_rel != nullptr);
        REQUIRE(mul_rel->get_arity() == 3);
        REQUIRE(mul_rel->size() == 2);
        REQUIRE(mul_rel->get_operator_symbol() == mul_sym);
        
        // Test tuple access
        REQUIRE(add_rel->get(0, 0) == 1);
        REQUIRE(add_rel->get(0, 1) == 2);
        REQUIRE(add_rel->get(0, 2) == 3);
        
        std::vector<id_t> first_tuple = add_rel->get_tuple(0);
        REQUIRE(first_tuple == std::vector<id_t>{1, 2, 3});
    }
    
    SECTION("Test dump functionality") {
        // Create a simple relation
        db.add_relation(add_sym, 2);
        db.add_tuple(add_sym, {10, 20});
        db.add_tuple(add_sym, {30, 40});
        db.add_tuple(add_sym, {50, 60});
        
        const Relation* rel = db.get_relation(add_sym);
        REQUIRE(rel != nullptr);
        
        // Test dump output
        std::ostringstream oss;
        rel->dump(oss, symbol_table);
        
        std::string expected = "relation add with arity 2\n10 20\n30 40\n50 60\n";
        REQUIRE(oss.str() == expected);
    }
}