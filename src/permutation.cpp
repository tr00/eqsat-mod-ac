#include "permutation.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

uint32_t factorial(int n) {
    if (n < 0 || n > 12) {
        throw std::invalid_argument("Factorial input must be between 0 and 12");
    }
    
    uint32_t result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

bool is_valid_permutation(const std::vector<uint32_t>& perm) {
    if (perm.empty()) {
        return true;
    }
    
    // Find min and max elements
    uint32_t min_elem = *std::min_element(perm.begin(), perm.end());
    uint32_t max_elem = *std::max_element(perm.begin(), perm.end());
    
    // Check if range is correct
    if (max_elem - min_elem + 1 != perm.size()) {
        return false;
    }
    
    // Check for duplicates using a set
    std::unordered_set<uint32_t> seen;
    for (uint32_t elem : perm) {
        if (seen.count(elem) > 0) {
            return false;
        }
        seen.insert(elem);
    }
    
    return true;
}

uint32_t permutation_to_index(const std::vector<uint32_t>& perm) {
    if (!is_valid_permutation(perm)) {
        throw std::invalid_argument("Input is not a valid permutation");
    }
    
    if (perm.empty()) {
        return 0;
    }
    
    size_t n = perm.size();
    if (n > 12) {
        throw std::invalid_argument("Permutation too large (max size is 12)");
    }
    
    // Create sorted copy of elements to use as reference
    std::vector<uint32_t> available = perm;
    std::sort(available.begin(), available.end());
    
    uint32_t index = 0;
    uint32_t fact = factorial(n - 1);
    
    for (size_t i = 0; i < n - 1; ++i) {
        // Find position of perm[i] in remaining available elements
        auto it = std::find(available.begin(), available.end(), perm[i]);
        size_t pos = std::distance(available.begin(), it);
        
        // Add contribution to index
        index += pos * fact;
        
        // Remove this element from available list
        available.erase(it);
        
        // Update factorial for next position
        if (i < n - 2) {
            fact /= (n - 1 - i);
        }
    }
    
    return index;
}

std::vector<uint32_t> index_to_permutation(uint32_t index, const std::vector<uint32_t>& elements) {
    if (elements.empty()) {
        if (index == 0) {
            return {};
        } else {
            throw std::invalid_argument("Index too large for empty element set");
        }
    }
    
    size_t n = elements.size();
    if (n > 12) {
        throw std::invalid_argument("Element set too large (max size is 12)");
    }
    
    // Check if index is valid for this number of elements
    uint32_t max_index = factorial(n);
    if (index >= max_index) {
        throw std::invalid_argument("Index too large for given number of elements");
    }
    
    // Create sorted copy of elements to work with
    std::vector<uint32_t> available = elements;
    std::sort(available.begin(), available.end());
    
    std::vector<uint32_t> result;
    result.reserve(n);
    
    uint32_t fact = factorial(n - 1);
    uint32_t remaining_index = index;
    
    for (size_t i = 0; i < n; ++i) {
        // Calculate position in remaining elements
        size_t pos = remaining_index / fact;
        
        // Add the element at this position
        result.push_back(available[pos]);
        
        // Remove this element from available list
        available.erase(available.begin() + pos);
        
        // Update remaining index and factorial for next iteration
        if (i < n - 1) {
            remaining_index %= fact;
            fact /= (n - 1 - i);
        }
    }
    
    return result;
}