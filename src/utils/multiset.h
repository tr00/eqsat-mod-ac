#pragma once

#include <algorithm>
#include <functional>
#include <utility>

#include "../id.h"
#include "utils/hash.h"
#include "utils/vec.h"

namespace eqsat
{

class MultisetSupport;

struct MultisetFingerprint
{
    uint64_t fingerprint = SEED2;

    MultisetFingerprint()
    {
    }

    uint64_t hash() const noexcept
    {
        return fingerprint;
    }

    void insert(id_t x)
    {
        fingerprint = eqsat::addmodp(fingerprint, eqsat::hashmodp(x));
    }

    void insert(id_t x, unsigned count)
    {
        fingerprint = eqsat::addmodp(fingerprint, eqsat::mulmodp(eqsat::hashmodp(x), count));
    }

    void remove(id_t x)
    {
        fingerprint = eqsat::submodp(fingerprint, eqsat::hashmodp(x));
    }

    void reset()
    {
        fingerprint = SEED2;
    }
};

/**
 * @brief A multiset data structure that maintains element counts and supports efficient operations.
 *
 * This class implements a multiset (bag) where elements can appear multiple times. Elements are
 * stored in sorted order with their counts, enabling efficient lookups, insertions, and removals.
 *
 * The multiset uses an incremental hash function based on commutative operations modulo a prime,
 * allowing the hash to be updated efficiently during insertions and removals without recomputing
 * the entire hash from scratch.
 *
 * Implementation notes:
 * - Elements are stored as (id, count) pairs in sorted order by id
 * - Zero-count entries are intentionally preserved to optimize temporary removal/reinsertion
 *   patterns that occur during pattern matching
 * - Hash computation uses addition and multiplication modulo PRIME for commutative incremental updates
 * - All operations maintain the sorted invariant
 */
class Multiset
{
  private:
    Vec<std::pair<id_t, uint32_t>> data;
    size_t nelements;
    MultisetFingerprint fingerprint;

    static bool cmp(const std::pair<id_t, uint32_t>& pair, id_t val)
    {
        return pair.first < val;
    }

    /**
     * @brief Performs binary search to find the position of an id in the sorted data vector.
     *
     * @param id The element to search for
     * @return Iterator pointing to the element if found, or the insertion position if not found
     */
    auto find_pos(id_t id) noexcept
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

    auto find_pos(id_t id) const noexcept
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

    /**
     * @brief Constructs the multiset from a range of elements.
     *
     * Counts occurrences of each unique element in the range and stores them in sorted order.
     * Called by the constructors to initialize the multiset.
     *
     * @param begin Iterator to the start of the range
     * @param end Iterator to the end of the range
     */
    void construct(Vec<id_t>::const_iterator begin, Vec<id_t>::const_iterator end)
    {
        data.reserve(end - begin);

        for (; begin < end; ++begin)
        {
            auto id = *begin;
            auto it = find_pos(id);

            if (it != data.end() && it->first == id)
                ++it->second;
            else
                data.insert(it, {id, 1});
        }

        data.shrink_to_fit();

        rehash();
    }

    /**
     * @brief Recomputes the fingerprint hash from scratch.
     *
     * Used after bulk modifications (e.g., map operations) where incremental updates would be
     * inefficient. Computes hash as: secret + Σ(hash(value) * count) mod PRIME for all entries.
     * Zero-count entries are skipped during rehashing.
     */
    void rehash()
    {
        fingerprint.reset();

        for (const auto& [value, count] : data)
            if (count > 0)
                fingerprint.insert(value, count);
    }

    friend class MultisetSupport;
    friend class RelationAC;

  public:
    /**
     * @brief Constructs an empty multiset.
     */
    Multiset()
        : data()
        , nelements(0)
        , fingerprint()
    {
    }

    /**
     * @brief Constructs a multiset from a vector of elements.
     *
     * Elements are counted and stored in sorted order. Duplicate elements in the input
     * increase the count of that element.
     *
     * @param vec Vector of elements to initialize the multiset
     */
    Multiset(const Vec<id_t>& vec)
        : data()
        , nelements(vec.size())
        , fingerprint()
    {
        construct(vec.cbegin(), vec.cend());
    }

    /**
     * @brief Constructs a multiset from a range of elements.
     *
     * Elements are counted and stored in sorted order. Duplicate elements in the input
     * increase the count of that element.
     *
     * @param begin Iterator to the start of the range
     * @param end Iterator to the end of the range
     */
    Multiset(Vec<id_t>::const_iterator begin, Vec<id_t>::const_iterator end)
        : data()
        , nelements(end - begin)
        , fingerprint()
    {
        construct(begin, end);
    }

    /**
     * @brief Tests equality between two multisets.
     *
     * Two multisets are equal if they contain the same elements with the same counts.
     * Uses hash comparison as an early-exit optimization before performing full comparison.
     *
     * @param other The multiset to compare against
     * @return true if the multisets are equal, false otherwise
     */
    [[nodiscard]] bool operator==(const Multiset& other) const
    {
        if (this->hash() != other.hash())
            return false;

        if (this->size() != other.size())
            return false;

        for (const auto& [value, count] : this->data)
            if (other.count(value) != count)
                return false;

        for (const auto& [value, count] : other.data)
            if (this->count(value) != count)
                return false;

        return true;
    }

    /**
     * @brief Checks if this multiset includes another as a submultiset.
     *
     * Returns true if for every element in 'other', this multiset contains at least as many
     * occurrences of that element.
     *
     * @param other The potential submultiset to check
     * @return true if 'other' is a submultiset of this multiset, false otherwise
     */
    [[nodiscard]] bool includes(const Multiset& other) const
    {
        if (other.nelements > this->nelements)
            return false;

        for (const auto& [value, count] : other.data)
            if (this->count(value) < count)
                return false;

        return true;
    }

    /**
     * @brief Inserts multiple occurrences of an element into the multiset.
     *
     * Increments the count of the specified element by 'count'. Updates the hash incrementally
     * without requiring a full rehash.
     *
     * @param id The element to insert
     * @param count The number of occurrences to add
     */
    void insert(id_t id, uint32_t count)
    {
        if (count == 0)
            return;

        auto it = find_pos(id);
        if (it != data.end() && it->first == id)
        {
            it->second += count;
        }
        else
        {
            data.insert(it, {id, count});
        }

        nelements += count;

        fingerprint.insert(id, count);
    }

    /**
     * @brief Inserts a single occurrence of an element into the multiset.
     *
     * @param id The element to insert
     */
    void insert(id_t id)
    {
        insert(id, 1);
    }

    /**
     * @brief Removes a single occurrence of an element from the multiset.
     *
     * Decrements the count of the element by one if it exists and has count > 0.
     * Zero-count entries are intentionally preserved in the data structure to optimize
     * temporary removal patterns during pattern matching.
     *
     * @param id The element to remove
     */
    void remove(id_t id)
    {
        auto it = find_pos(id);
        if (it != data.end() && it->first == id && it->second > 0)
        {
            it->second--;
            nelements--;

            fingerprint.remove(id);
        }
    }

    /**
     * @brief Checks if an element is present in the multiset with non-zero count.
     *
     * @param id The element to check
     * @return true if the element has count > 0, false otherwise
     */
    bool contains(id_t id) const
    {
        auto it = find_pos(id);
        return it != data.end() && it->first == id && it->second > 0;
    }

    /**
     * @brief Returns the count of an element in the multiset.
     *
     * @param id The element to query
     * @return The number of occurrences of the element (0 if not present)
     */
    [[nodiscard]] uint32_t count(id_t id) const
    {
        auto it = find_pos(id);
        return (it != data.end() && it->first == id) ? it->second : 0;
    }

    /**
     * @brief Removes all elements from the multiset.
     */
    void clear()
    {
        data.clear();
        nelements = 0;
        fingerprint.reset();
    }

    /**
     * @brief Returns the total number of elements including multiplicities.
     *
     * @return The sum of all element counts
     */
    size_t size() const
    {
        return nelements;
    }

    size_t unique_size() const
    {
        // not entirely accurate but always larger than
        // the actual number of unique elements
        return data.size();
    }

    /**
     * @brief Checks if the multiset contains no elements with positive count.
     *
     * @return true if the multiset is empty, false otherwise
     */
    [[nodiscard]] bool empty() const
    {
        return data.empty();
    }

    /**
     * @brief Applies a transformation function to all elements in the multiset.
     *
     * Maps each element id to a new id using the provided function. After mapping, elements
     * with the same new id have their counts merged. The multiset is re-sorted and rehashed
     * after the transformation.
     *
     * @param f Function that maps each element id to a new id
     * @return true if any element was changed, false if all elements mapped to themselves
     */
    bool map(std::function<id_t(id_t)> f)
    {
        bool changed = false;

        for (auto& [value, _] : data)
        {
            id_t newval = f(value);
            if (newval != value)
            {
                value = newval;
                changed = true;
            }
        }

        if (!changed)
            return false;

        std::sort(data.begin(), data.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

        if (data.empty())
            return true;

        size_t j = 0;
        for (size_t i = 1; i < data.size(); ++i)
        {
            if (data[j].first == data[i].first)
            {
                data[j].second += data[i].second;
            }
            else
            {
                ++j;
                if (j != i)
                {
                    data[j] = data[i];
                }
            }
        }

        data.resize(j + 1);
        rehash();

        return true;
    }

    /**
     * @brief Returns the hash fingerprint of the multiset.
     *
     * The hash is computed using a commutative function based on addition and multiplication
     * modulo a prime, making it order-independent and suitable for incremental updates.
     * The hash value is maintained incrementally during insertions and removals.
     *
     * @return The 64-bit hash fingerprint
     */
    uint64_t hash() const noexcept
    {
        return fingerprint.hash();
    }

    /**
     * @brief Computes the multiset difference between this and another multiset.
     *
     * Returns a new multiset containing elements from this multiset with their counts reduced
     * by the counts in 'other'. Elements with zero or negative difference are omitted.
     *
     * @param other The multiset to subtract
     * @return A new multiset representing this \ other
     */
    [[nodiscard]] Multiset msetdiff(const Multiset& other) const
    {
        Multiset diff;

        for (const auto& [value, count] : data)
        {
            uint32_t other_count = other.count(value);
            if (count > other_count)
            {
                diff.insert(value, count - other_count);
            }
        }

        return diff;
    }

    void insert_all(const Multiset& other)
    {
        for (const auto& [value, count] : other.data)
            insert(value, count);
    }

    /**
     * @brief Collects all elements into a vector, respecting multiplicities.
     *
     * Returns a vector where each element appears according to its count in the multiset.
     * The order of elements in the output vector reflects the sorted internal representation.
     *
     * @return A vector containing all elements with their multiplicities
     */
    [[nodiscard]] Vec<id_t> collect() const
    {
        Vec<id_t> vec;
        vec.reserve(nelements);

        for (const auto& [value, count] : data)
            for (uint32_t i = 0; i < count; ++i)
                vec.push_back(value);

        return vec;
    }
};

} // namespace eqsat
