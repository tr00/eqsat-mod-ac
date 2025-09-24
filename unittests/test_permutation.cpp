#include <catch2/catch_test_macros.hpp>
#include <numeric>

#include "permutation.h"

TEST_CASE("Factorial function", "[permutation]") {
    SECTION("Basic factorial calculations") {
        REQUIRE(factorial(0) == 1);
        REQUIRE(factorial(1) == 1);
        REQUIRE(factorial(2) == 2);
        REQUIRE(factorial(3) == 6);
        REQUIRE(factorial(4) == 24);
        REQUIRE(factorial(5) == 120);
        REQUIRE(factorial(6) == 720);
    }
    
    SECTION("Edge cases") {
        REQUIRE_THROWS_AS(factorial(-1), std::invalid_argument);
        REQUIRE_THROWS_AS(factorial(13), std::invalid_argument);
        REQUIRE(factorial(12) == 479001600);
    }
}

TEST_CASE("is_valid_permutation function", "[permutation]") {
    SECTION("Valid permutations") {
        REQUIRE(is_valid_permutation({}));
        REQUIRE(is_valid_permutation({0}));
        REQUIRE(is_valid_permutation({0, 1}));
        REQUIRE(is_valid_permutation({1, 0}));
        REQUIRE(is_valid_permutation({0, 1, 2}));
        REQUIRE(is_valid_permutation({2, 0, 1}));
        REQUIRE(is_valid_permutation({1, 2, 3})); // Non-zero based
    }
    
    SECTION("Invalid permutations") {
        REQUIRE_FALSE(is_valid_permutation({0, 0})); // Duplicate
        REQUIRE_FALSE(is_valid_permutation({0, 2})); // Missing 1
        REQUIRE_FALSE(is_valid_permutation({1, 2, 2})); // Duplicate
        REQUIRE_FALSE(is_valid_permutation({0, 1, 3})); // Missing 2
    }
}

TEST_CASE("permutation_to_index basic cases", "[permutation]") {
    SECTION("Empty permutation") {
        std::vector<uint32_t> perm = {};
        REQUIRE(permutation_to_index(perm) == 0);
    }
    
    SECTION("Single element") {
        std::vector<uint32_t> perm = {0};
        REQUIRE(permutation_to_index(perm) == 0);
    }
    
    SECTION("Two elements") {
        REQUIRE(permutation_to_index({0, 1}) == 0);
        REQUIRE(permutation_to_index({1, 0}) == 1);
    }
    
    SECTION("Three elements - all permutations") {
        REQUIRE(permutation_to_index({0, 1, 2}) == 0);
        REQUIRE(permutation_to_index({0, 2, 1}) == 1);
        REQUIRE(permutation_to_index({1, 0, 2}) == 2);
        REQUIRE(permutation_to_index({1, 2, 0}) == 3);
        REQUIRE(permutation_to_index({2, 0, 1}) == 4);
        REQUIRE(permutation_to_index({2, 1, 0}) == 5);
    }
}

TEST_CASE("index_to_permutation basic cases", "[permutation]") {
    SECTION("Empty elements") {
        std::vector<uint32_t> elements = {};
        REQUIRE(index_to_permutation(0, elements) == std::vector<uint32_t>{});
        REQUIRE_THROWS_AS(index_to_permutation(1, elements), std::invalid_argument);
    }
    
    SECTION("Single element") {
        std::vector<uint32_t> elements = {5};
        REQUIRE(index_to_permutation(0, elements) == std::vector<uint32_t>{5});
        REQUIRE_THROWS_AS(index_to_permutation(1, elements), std::invalid_argument);
    }
    
    SECTION("Two elements") {
        std::vector<uint32_t> elements = {0, 1};
        REQUIRE(index_to_permutation(0, elements) == std::vector<uint32_t>{0, 1});
        REQUIRE(index_to_permutation(1, elements) == std::vector<uint32_t>{1, 0});
        REQUIRE_THROWS_AS(index_to_permutation(2, elements), std::invalid_argument);
    }
    
    SECTION("Three elements - all indices") {
        std::vector<uint32_t> elements = {0, 1, 2};
        REQUIRE(index_to_permutation(0, elements) == std::vector<uint32_t>{0, 1, 2});
        REQUIRE(index_to_permutation(1, elements) == std::vector<uint32_t>{0, 2, 1});
        REQUIRE(index_to_permutation(2, elements) == std::vector<uint32_t>{1, 0, 2});
        REQUIRE(index_to_permutation(3, elements) == std::vector<uint32_t>{1, 2, 0});
        REQUIRE(index_to_permutation(4, elements) == std::vector<uint32_t>{2, 0, 1});
        REQUIRE(index_to_permutation(5, elements) == std::vector<uint32_t>{2, 1, 0});
        REQUIRE_THROWS_AS(index_to_permutation(6, elements), std::invalid_argument);
    }
}

TEST_CASE("Round-trip conversions", "[permutation]") {
    SECTION("All permutations of size 3") {
        std::vector<uint32_t> elements = {0, 1, 2};
        
        // Test all possible permutations
        std::vector<std::vector<uint32_t>> all_perms = {
            {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, 
            {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
        };
        
        for (size_t i = 0; i < all_perms.size(); ++i) {
            // perm -> index -> perm
            uint32_t index = permutation_to_index(all_perms[i]);
            std::vector<uint32_t> recovered = index_to_permutation(index, elements);
            REQUIRE(recovered == all_perms[i]);
            REQUIRE(index == i);
            
            // index -> perm -> index  
            std::vector<uint32_t> perm = index_to_permutation(i, elements);
            uint32_t recovered_index = permutation_to_index(perm);
            REQUIRE(recovered_index == i);
            REQUIRE(perm == all_perms[i]);
        }
    }
    
    SECTION("All permutations of size 4") {
        std::vector<uint32_t> elements = {0, 1, 2, 3};
        
        // Test round-trip for all 24 permutations
        for (uint32_t i = 0; i < 24; ++i) {
            std::vector<uint32_t> perm = index_to_permutation(i, elements);
            uint32_t recovered_index = permutation_to_index(perm);
            REQUIRE(recovered_index == i);
        }
    }
}

TEST_CASE("Non-zero based permutations", "[permutation]") {
    SECTION("Permutation starting from 1") {
        std::vector<uint32_t> perm1 = {1, 2, 3};
        std::vector<uint32_t> perm2 = {3, 1, 2};
        
        uint32_t index1 = permutation_to_index(perm1);
        uint32_t index2 = permutation_to_index(perm2);
        
        REQUIRE(index1 == 0); // First permutation
        REQUIRE(index2 == 4); // Same relative order as {2, 0, 1}
        
        // Round-trip test
        std::vector<uint32_t> elements = {1, 2, 3};
        std::vector<uint32_t> recovered1 = index_to_permutation(index1, elements);
        std::vector<uint32_t> recovered2 = index_to_permutation(index2, elements);
        
        REQUIRE(recovered1 == perm1);
        REQUIRE(recovered2 == perm2);
    }
    
    SECTION("Arbitrary range permutation") {
        std::vector<uint32_t> elements = {5, 6, 7};
        
        // Test a few permutations
        std::vector<uint32_t> perm1 = {5, 6, 7}; // index 0
        std::vector<uint32_t> perm2 = {7, 5, 6}; // index 4
        
        REQUIRE(permutation_to_index(perm1) == 0);
        REQUIRE(permutation_to_index(perm2) == 4);
        
        REQUIRE(index_to_permutation(0, elements) == perm1);
        REQUIRE(index_to_permutation(4, elements) == perm2);
    }
}

TEST_CASE("Error handling", "[permutation]") {
    SECTION("permutation_to_index errors") {
        REQUIRE_THROWS_AS(permutation_to_index({0, 0}), std::invalid_argument);
        REQUIRE_THROWS_AS(permutation_to_index({0, 2}), std::invalid_argument);
        
        // Test large permutation (> 12 elements)
        std::vector<uint32_t> large_perm(13);
        std::iota(large_perm.begin(), large_perm.end(), 0);
        REQUIRE_THROWS_AS(permutation_to_index(large_perm), std::invalid_argument);
    }
    
    SECTION("index_to_permutation errors") {
        std::vector<uint32_t> elements = {0, 1, 2};
        REQUIRE_THROWS_AS(index_to_permutation(6, elements), std::invalid_argument);
        
        // Test large element set (> 12 elements)
        std::vector<uint32_t> large_elements(13);
        std::iota(large_elements.begin(), large_elements.end(), 0);
        REQUIRE_THROWS_AS(index_to_permutation(0, large_elements), std::invalid_argument);
    }
}

TEST_CASE("Performance test with larger permutations", "[permutation]") {
    SECTION("Size 6 permutations") {
        std::vector<uint32_t> elements = {0, 1, 2, 3, 4, 5};
        
        // Test first and last permutations
        std::vector<uint32_t> first_perm = index_to_permutation(0, elements);
        std::vector<uint32_t> last_perm = index_to_permutation(719, elements); // 6! - 1
        
        REQUIRE(first_perm == std::vector<uint32_t>{0, 1, 2, 3, 4, 5});
        REQUIRE(last_perm == std::vector<uint32_t>{5, 4, 3, 2, 1, 0});
        
        // Round-trip tests
        REQUIRE(permutation_to_index(first_perm) == 0);
        REQUIRE(permutation_to_index(last_perm) == 719);
    }
}