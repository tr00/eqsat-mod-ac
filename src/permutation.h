#pragma once

#include <vector>
#include <cstdint>

/**
 * @brief Permutation utilities for converting between permutation vectors and indices
 * 
 * This module provides efficient conversion between permutation vectors and their
 * lexicographic indices using the factorial number system (Lehmer code).
 * 
 * For example, with elements [0,1,2], the permutations and their indices are:
 * - [0,1,2] -> index 0
 * - [0,2,1] -> index 1  
 * - [1,0,2] -> index 2
 * - [1,2,0] -> index 3
 * - [2,0,1] -> index 4
 * - [2,1,0] -> index 5
 */

/**
 * @brief Convert a permutation vector to its lexicographic index
 * 
 * Converts a permutation of elements to its corresponding lexicographic index
 * using the factorial number system. The permutation must be a valid permutation
 * of consecutive integers starting from some base value.
 * 
 * @param perm The permutation vector to convert
 * @return The lexicographic index of the permutation
 * 
 * @throws std::invalid_argument if perm is not a valid permutation
 * 
 * Example:
 * ```cpp
 * std::vector<uint32_t> perm = {2, 0, 1};
 * uint32_t index = permutation_to_index(perm); // Returns 4
 * ```
 */
uint32_t permutation_to_index(const std::vector<uint32_t>& perm);

/**
 * @brief Convert a lexicographic index to its corresponding permutation vector
 * 
 * Converts a lexicographic index back to the corresponding permutation of the
 * given elements using the factorial number system.
 * 
 * @param index The lexicographic index to convert
 * @param elements The base elements to permute (must be sorted)
 * @return The permutation vector corresponding to the index
 * 
 * @throws std::invalid_argument if index is too large for the given elements
 * 
 * Example:
 * ```cpp
 * std::vector<uint32_t> elements = {0, 1, 2};
 * std::vector<uint32_t> perm = index_to_permutation(4, elements); // Returns {2, 0, 1}
 * ```
 */
std::vector<uint32_t> index_to_permutation(uint32_t index, const std::vector<uint32_t>& elements);

/**
 * @brief Calculate the factorial of n
 * 
 * Helper function to compute n! efficiently for small values of n.
 * Used internally by permutation conversion functions.
 * 
 * @param n The number to calculate factorial for
 * @return n! (n factorial)
 * 
 * @throws std::invalid_argument if n is too large (> 12)
 */
uint32_t factorial(int n);

/**
 * @brief Check if a vector is a valid permutation
 * 
 * Validates that the given vector contains each integer from min to max
 * exactly once, making it a valid permutation.
 * 
 * @param perm The vector to check
 * @return true if perm is a valid permutation, false otherwise
 * 
 * Example:
 * ```cpp
 * std::vector<uint32_t> valid = {2, 0, 1};
 * std::vector<uint32_t> invalid = {0, 0, 1};
 * bool is_valid1 = is_valid_permutation(valid);   // Returns true
 * bool is_valid2 = is_valid_permutation(invalid); // Returns false
 * ```
 */
bool is_valid_permutation(const std::vector<uint32_t>& perm);