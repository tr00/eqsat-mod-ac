
#pragma once

#include <algorithm>
#include <functional>
#include <utility>

#include "../id.h"
#include "../utils/vec.h"
#include "utils/hash.h"

class MultisetSupport;

class Multiset
{
  private:
    // vector of (id, count)
    // ids should be unique and sorted
    Vec<std::pair<id_t, uint32_t>> data;

    static bool cmp(const std::pair<id_t, uint32_t>& pair, id_t val)
    {
        return pair.first < val;
    }

    // Binary search to find position of id
    auto find_pos(id_t id) noexcept
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

    auto find_pos(id_t id) const noexcept
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

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
    }

    friend class MultisetSupport;
    friend class RelationAC;

  public:
    Multiset()
        : data()
    {
    }

    Multiset(const Vec<id_t>& vec)
        : data()
    {
        construct(vec.cbegin(), vec.cend());
    }

    Multiset(Vec<id_t>::const_iterator begin, Vec<id_t>::const_iterator end)
        : data()
    {
        construct(begin, end);
    }

    [[nodiscard]] bool operator==(const Multiset& other) const
    {
        if (this->size() != other.size()) return false;
        if (this->data.size() != other.data.size()) return false;

        for (const auto& [value, count] : this->data)
            if (other.count(value) != count) return false;

        for (const auto& [value, count] : other.data)
            if (this->count(value) != count) return false;

        return true;
    }

    [[nodiscard]] bool includes(const Multiset& other) const
    {
        for (const auto& [value, count] : this->data)
            if (other.count(value) < count) return false;

        return true;
    }

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
            // Skip if count <= other_count (would underflow)
        }

        return diff;
    }

    void insert(id_t id, uint32_t count)
    {
        auto it = find_pos(id);
        if (it != data.end() && it->first == id)
        {
            it->second += count;
        }
        else
        {
            data.insert(it, {id, count});
        }
    }

    void insert(id_t id)
    {
        insert(id, 1);
    }

    void remove(id_t id)
    {
        auto it = find_pos(id);
        if (it != data.end() && it->first == id && it->second > 0)
        {
            it->second--;
        }
    }

    bool contains(id_t id) const
    {
        auto it = find_pos(id);
        return it != data.end() && it->first == id && it->second > 0;
    }

    [[nodiscard]] uint32_t count(id_t id) const
    {
        auto it = find_pos(id);
        return (it != data.end() && it->first == id) ? it->second : 0;
    }

    void clear()
    {
        data.clear();
    }

    // Note: this is an overestimate of the support!
    size_t size() const
    {
        return data.size();
    }

    [[nodiscard]] bool empty() const
    {
        return data.empty();
    }

    void map(std::function<id_t(id_t)> f)
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

        if (!changed) return;

        std::sort(data.begin(), data.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

        // merge entries with same id
        if (data.empty()) return;

        size_t write_idx = 0;
        for (size_t read_idx = 1; read_idx < data.size(); ++read_idx)
        {
            if (data[write_idx].first == data[read_idx].first)
            {
                // Same id - merge counts
                data[write_idx].second += data[read_idx].second;
            }
            else
            {
                // Different id - move to next write position
                ++write_idx;
                if (write_idx != read_idx)
                {
                    data[write_idx] = data[read_idx];
                }
            }
        }

        data.resize(write_idx + 1);
    }

    uint64_t hash() const noexcept
    {
        uint64_t h = SEED;

        for (const auto& [a, b] : data)
        {
            h = eqsat::mix64(h, a);
            h = eqsat::mix64(h, b);
        }

        return h;
    }
};
