
#pragma once

#include "id.h"
#include <algorithm>
#include <functional>
#include <utility>

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
    auto find_pos(id_t id)
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

    auto find_pos(id_t id) const
    {
        return std::lower_bound(data.begin(), data.end(), id, cmp);
    }

    friend class MultisetSupport;

  public:
    Multiset()
        : data()
    {
    }

    Multiset(const Vec<id_t>& vec)
        : data()
    {
        data.reserve(vec.size());

        for (id_t id : vec)
        {
            auto it = find_pos(id);

            if (it != data.end() && it->first == id)
                ++it->second;
            else
                data.insert(it, {id, 1});
        }
    }

    bool operator==(Multiset const& other) const
    {
        if (this->size() != other.size()) return false;

        for (auto [value, count] : this->data)
            if (other.count(value) != count) return false;

        for (auto [value, count] : other.data)
            if (this->count(value) != count) return false;

        return true;
    }

    void insert(id_t id)
    {
        auto it = find_pos(id);
        if (it != data.end() && it->first == id)
        {
            it->second++;
        }
        else
        {
            data.insert(it, {id, 1});
        }
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

    // Get count of element
    uint32_t count(id_t id) const
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

    bool empty() const
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

        // TODO: merge entries with same id
    }

    // Hash function for use in hash maps
    size_t hash() const
    {
        size_t h = 0;
        for (const auto& [id, count] : data)
        {
            // Combine hash of id and count
            size_t pair_hash = std::hash<id_t>{}(id) ^ (std::hash<uint32_t>{}(count) << 1);
            h ^= pair_hash + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};
