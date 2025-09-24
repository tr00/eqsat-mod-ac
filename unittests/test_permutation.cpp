#include <catch2/catch_test_macros.hpp>
#include <numeric>
#include <algorithm>

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

TEST_CASE("apply_permutation with precomputed indices", "[permutation]") {
    SECTION("Basic functionality") {
        std::vector<uint32_t> perm_indices = {2, 0, 1};  // [2,0,1] permutation
        std::vector<uint32_t> vec = {10, 20, 30};
        apply_permutation(perm_indices, vec);
        REQUIRE(vec == std::vector<uint32_t>{30, 10, 20});
    }

    SECTION("Identity permutation") {
        std::vector<uint32_t> perm_indices = {0, 1, 2};
        std::vector<uint32_t> vec = {10, 20, 30};
        apply_permutation(perm_indices, vec);
        REQUIRE(vec == std::vector<uint32_t>{10, 20, 30});
    }

    SECTION("Reverse permutation") {
        std::vector<uint32_t> perm_indices = {2, 1, 0};
        std::vector<uint32_t> vec = {10, 20, 30};
        apply_permutation(perm_indices, vec);
        REQUIRE(vec == std::vector<uint32_t>{30, 20, 10});
    }

    SECTION("Empty vectors") {
        std::vector<uint32_t> perm_indices = {};
        std::vector<uint32_t> vec = {};
        apply_permutation(perm_indices, vec);
        REQUIRE(vec.empty());
    }

    SECTION("Single element") {
        std::vector<uint32_t> perm_indices = {0};
        std::vector<uint32_t> vec = {42};
        apply_permutation(perm_indices, vec);
        REQUIRE(vec == std::vector<uint32_t>{42});
    }
}

TEST_CASE("apply_permutation with precomputed indices - error handling", "[permutation]") {
    SECTION("Size mismatch") {
        std::vector<uint32_t> perm_indices = {0, 1};
        std::vector<uint32_t> vec = {10, 20, 30};
        REQUIRE_THROWS_AS(apply_permutation(perm_indices, vec), std::invalid_argument);

        std::vector<uint32_t> perm_indices2 = {0, 1, 2};
        std::vector<uint32_t> vec2 = {10, 20};
        REQUIRE_THROWS_AS(apply_permutation(perm_indices2, vec2), std::invalid_argument);
    }

    SECTION("Index out of bounds") {
        std::vector<uint32_t> perm_indices = {0, 3, 1};  // 3 is out of bounds for size 3
        std::vector<uint32_t> vec = {10, 20, 30};
        REQUIRE_THROWS_AS(apply_permutation(perm_indices, vec), std::out_of_range);

        std::vector<uint32_t> perm_indices2 = {0, 1, 5};  // 5 is out of bounds
        std::vector<uint32_t> vec2 = {10, 20, 30};
        REQUIRE_THROWS_AS(apply_permutation(perm_indices2, vec2), std::out_of_range);
    }
}

TEST_CASE("apply_permutation consistency between versions", "[permutation]") {
    SECTION("Both versions should produce identical results") {
        std::vector<uint32_t> original = {100, 200, 300, 400};

        // Test all 24 permutations of 4 elements
        for (uint32_t perm_idx = 0; perm_idx < 24; ++perm_idx) {
            // Version 1: Using permutation index
            std::vector<uint32_t> vec1 = original;
            apply_permutation(perm_idx, vec1);

            // Version 2: Using precomputed indices
            std::vector<uint32_t> indices = {0, 1, 2, 3};
            std::vector<uint32_t> perm_indices = index_to_permutation(perm_idx, indices);
            std::vector<uint32_t> vec2 = original;
            apply_permutation(perm_indices, vec2);

            // Both should produce the same result
            REQUIRE(vec1 == vec2);
        }
    }
}

TEST_CASE("apply_permutation performance comparison setup", "[permutation]") {
    SECTION("Verify precomputed version works for multiple applications") {
        std::vector<uint32_t> original = {10, 20, 30};

        // Precompute permutation indices once
        std::vector<uint32_t> indices = {0, 1, 2};
        std::vector<uint32_t> perm_indices = index_to_permutation(4, indices); // [2,0,1]

        // Apply to multiple vectors
        for (uint32_t i = 0; i < 5; ++i) {
            std::vector<uint32_t> vec = {10 + i, 20 + i, 30 + i};
            apply_permutation(perm_indices, vec);
            REQUIRE(vec == std::vector<uint32_t>{30 + i, 10 + i, 20 + i});
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

TEST_CASE("apply_permutation basic cases", "[permutation]") {
    SECTION("Empty vector") {
        std::vector<uint32_t> vec = {};
        apply_permutation(0, vec);
        REQUIRE(vec == std::vector<uint32_t>{});

        std::vector<uint32_t> vec2 = {};
        REQUIRE_THROWS_AS(apply_permutation(1, vec2), std::invalid_argument);
    }

    SECTION("Single element") {
        std::vector<uint32_t> vec = {42};
        apply_permutation(0, vec);
        REQUIRE(vec == std::vector<uint32_t>{42});

        std::vector<uint32_t> vec2 = {42};
        REQUIRE_THROWS_AS(apply_permutation(1, vec2), std::invalid_argument);
    }

    SECTION("Two elements") {
        std::vector<uint32_t> vec1 = {10, 20};
        apply_permutation(0, vec1);
        REQUIRE(vec1 == std::vector<uint32_t>{10, 20});

        std::vector<uint32_t> vec2 = {10, 20};
        apply_permutation(1, vec2);
        REQUIRE(vec2 == std::vector<uint32_t>{20, 10});

        std::vector<uint32_t> vec3 = {10, 20};
        REQUIRE_THROWS_AS(apply_permutation(2, vec3), std::invalid_argument);
    }

    SECTION("Three elements - all permutations") {
        // Index 0: [0,1,2] -> [100, 200, 300]
        std::vector<uint32_t> vec0 = {100, 200, 300};
        apply_permutation(0, vec0);
        REQUIRE(vec0 == std::vector<uint32_t>{100, 200, 300});

        // Index 1: [0,2,1] -> [100, 300, 200]
        std::vector<uint32_t> vec1 = {100, 200, 300};
        apply_permutation(1, vec1);
        REQUIRE(vec1 == std::vector<uint32_t>{100, 300, 200});

        // Index 2: [1,0,2] -> [200, 100, 300]
        std::vector<uint32_t> vec2 = {100, 200, 300};
        apply_permutation(2, vec2);
        REQUIRE(vec2 == std::vector<uint32_t>{200, 100, 300});

        // Index 3: [1,2,0] -> [200, 300, 100]
        std::vector<uint32_t> vec3 = {100, 200, 300};
        apply_permutation(3, vec3);
        REQUIRE(vec3 == std::vector<uint32_t>{200, 300, 100});

        // Index 4: [2,0,1] -> [300, 100, 200]
        std::vector<uint32_t> vec4 = {100, 200, 300};
        apply_permutation(4, vec4);
        REQUIRE(vec4 == std::vector<uint32_t>{300, 100, 200});

        // Index 5: [2,1,0] -> [300, 200, 100]
        std::vector<uint32_t> vec5 = {100, 200, 300};
        apply_permutation(5, vec5);
        REQUIRE(vec5 == std::vector<uint32_t>{300, 200, 100});

        std::vector<uint32_t> vec6 = {100, 200, 300};
        REQUIRE_THROWS_AS(apply_permutation(6, vec6), std::invalid_argument);
    }
}

TEST_CASE("apply_permutation consistency with index_to_permutation", "[permutation]") {
    SECTION("Four elements - verify consistency") {
        std::vector<uint32_t> vec = {7, 11, 13, 17};
        std::vector<uint32_t> indices = {0, 1, 2, 3};

        // Test several permutation indices
        for (uint32_t perm_index = 0; perm_index < 24; ++perm_index) {
            // Get the permutation of indices
            std::vector<uint32_t> perm_indices = index_to_permutation(perm_index, indices);

            // Apply it manually
            std::vector<uint32_t> expected;
            for (uint32_t idx : perm_indices) {
                expected.push_back(vec[idx]);
            }

            // Compare with apply_permutation result
            std::vector<uint32_t> result = vec;
            apply_permutation(perm_index, result);
            REQUIRE(result == expected);
        }
    }
}

TEST_CASE("apply_permutation with duplicate values", "[permutation]") {
    SECTION("Vector with duplicates") {
        std::vector<uint32_t> vec = {5, 5, 5};

        // All permutations should give the same result since all elements are identical
        for (uint32_t i = 0; i < 6; ++i) {
            std::vector<uint32_t> test_vec = vec;
            apply_permutation(i, test_vec);
            REQUIRE(test_vec == std::vector<uint32_t>{5, 5, 5});
        }
    }

    SECTION("Vector with some duplicates") {
        std::vector<uint32_t> vec = {1, 2, 1};

        std::vector<uint32_t> vec0 = vec; apply_permutation(0, vec0); REQUIRE(vec0 == std::vector<uint32_t>{1, 2, 1}); // [0,1,2]
        std::vector<uint32_t> vec1 = vec; apply_permutation(1, vec1); REQUIRE(vec1 == std::vector<uint32_t>{1, 1, 2}); // [0,2,1]
        std::vector<uint32_t> vec2 = vec; apply_permutation(2, vec2); REQUIRE(vec2 == std::vector<uint32_t>{2, 1, 1}); // [1,0,2]
        std::vector<uint32_t> vec3 = vec; apply_permutation(3, vec3); REQUIRE(vec3 == std::vector<uint32_t>{2, 1, 1}); // [1,2,0]
        std::vector<uint32_t> vec4 = vec; apply_permutation(4, vec4); REQUIRE(vec4 == std::vector<uint32_t>{1, 1, 2}); // [2,0,1]
        std::vector<uint32_t> vec5 = vec; apply_permutation(5, vec5); REQUIRE(vec5 == std::vector<uint32_t>{1, 2, 1}); // [2,1,0]
    }
}

TEST_CASE("apply_permutation error handling", "[permutation]") {
    SECTION("Index too large for vector size") {
        std::vector<uint32_t> vec = {1, 2, 3};
        std::vector<uint32_t> vec1 = vec; REQUIRE_THROWS_AS(apply_permutation(6, vec1), std::invalid_argument);
        std::vector<uint32_t> vec2 = vec; REQUIRE_THROWS_AS(apply_permutation(100, vec2), std::invalid_argument);
    }

    SECTION("Vector too large") {
        std::vector<uint32_t> large_vec(13, 0);
        REQUIRE_THROWS_AS(apply_permutation(0, large_vec), std::invalid_argument);
    }
}

TEST_CASE("apply_permutation with larger vectors", "[permutation]") {
    SECTION("Five elements - spot checks") {
        std::vector<uint32_t> vec = {2, 4, 6, 8, 10};

        // Test first permutation
        std::vector<uint32_t> vec0 = vec;
        apply_permutation(0, vec0);
        REQUIRE(vec0 == std::vector<uint32_t>{2, 4, 6, 8, 10});

        // Test last permutation (index 119 = 5! - 1)
        std::vector<uint32_t> vec119 = vec;
        apply_permutation(119, vec119);
        REQUIRE(vec119 == std::vector<uint32_t>{10, 8, 6, 4, 2});

        // Test a middle permutation
        std::vector<uint32_t> vec60 = vec;
        apply_permutation(60, vec60);
        REQUIRE(vec60.size() == 5);

        // Verify it's a valid permutation of the original elements
        std::vector<uint32_t> sorted_result = vec60;
        std::sort(sorted_result.begin(), sorted_result.end());
        REQUIRE(sorted_result == vec);
    }
}

TEST_CASE("apply_permutation round-trip with permutation_to_index", "[permutation]") {
    SECTION("Verify apply_permutation gives expected ordering") {
        std::vector<uint32_t> vec = {15, 25, 35};

        // For each permutation index, apply it and verify we get the expected reordering
        for (uint32_t perm_index = 0; perm_index < 6; ++perm_index) {
            std::vector<uint32_t> result = vec;
            apply_permutation(perm_index, result);

            // Convert the result positions back to a permutation of indices
            std::vector<uint32_t> result_indices;
            for (uint32_t val : result) {
                if (val == 15) result_indices.push_back(0);
                else if (val == 25) result_indices.push_back(1);
                else if (val == 35) result_indices.push_back(2);
            }

            // The permutation index of these indices should match our input
            uint32_t computed_index = permutation_to_index(result_indices);
            REQUIRE(computed_index == perm_index);
        }
    }
}
