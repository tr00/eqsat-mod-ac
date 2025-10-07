#pragma once

#include <cstdint>

#include "id.h"

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
 * Vec<uint32_t> perm = {2, 0, 1};
 * uint32_t index = permutation_to_index(perm); // Returns 4
 * ```
 */
uint32_t permutation_to_index(const Vec<uint32_t>& perm);

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
 * Vec<uint32_t> elements = {0, 1, 2};
 * Vec<uint32_t> perm = index_to_permutation(4, elements); // Returns {2, 0, 1}
 * ```
 */
Vec<uint32_t> index_to_permutation(uint32_t index, const Vec<uint32_t>& elements);

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
 * Vec<uint32_t> valid = {2, 0, 1};
 * Vec<uint32_t> invalid = {0, 0, 1};
 * bool is_valid1 = is_valid_permutation(valid);   // Returns true
 * bool is_valid2 = is_valid_permutation(invalid); // Returns false
 * ```
 */
bool is_valid_permutation(const Vec<uint32_t>& perm);

/**
 * @brief Apply a permutation to a vector in-place using its lexicographic index
 *
 * Takes a lexicographic permutation index and applies the corresponding
 * permutation to the given vector elements in-place. The permutation is determined
 * by treating the vector indices as elements to permute.
 *
 * @param index The lexicographic index of the permutation to apply
 * @param vec The vector to permute in-place
 *
 * @throws std::invalid_argument if index is too large for the vector size
 *
 * Example:
 * ```cpp
 * Vec<uint32_t> vec = {10, 20, 30};
 * apply_permutation(4, vec); // vec becomes {30, 10, 20}
 * // Index 4 corresponds to permutation [2,0,1], so vec[2], vec[0], vec[1]
 * ```
 */
void apply_permutation(uint32_t index, Vec<uint32_t>& vec);

/**
 * @brief Apply a permutation to a vector in-place using precomputed indices
 *
 * More efficient version when the same permutation is applied multiple times.
 * Takes precomputed permutation indices and applies them to reorder the vector
 * elements in-place.
 *
 * @param perm_indices The permutation indices (e.g., [2,0,1] means vec[2], vec[0], vec[1])
 * @param vec The vector to permute in-place
 *
 * @throws std::invalid_argument if perm_indices size doesn't match vec size
 * @throws std::out_of_range if any index in perm_indices is out of bounds
 *
 * Example:
 * ```cpp
 * Vec<uint32_t> perm_indices = {2, 0, 1};  // Precomputed once
 * Vec<uint32_t> vec = {10, 20, 30};
 * apply_permutation(perm_indices, vec); // vec becomes {30, 10, 20}
 * ```
 */
void apply_permutation(const Vec<uint32_t>& perm_indices, Vec<uint32_t>& vec);
