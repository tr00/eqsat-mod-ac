
#pragma once

#include "id.h"
#include <algorithm>

class MultisetSupport;

class Multiset
{
  private:
    // vector of (id, count)
    // ids should be unique and sorted
    Vec<std::pair<id_t, uint32_t>> data;

    // Binary search to find position of id
    auto find_pos(id_t id)
    {
        return std::lower_bound(data.begin(), data.end(), id,
                                [](const auto& pair, id_t val) { return pair.first < val; });
    }

    auto find_pos(id_t id) const
    {
        return std::lower_bound(data.begin(), data.end(), id,
                                [](const auto& pair, id_t val) { return pair.first < val; });
    }

    friend class MultisetSupport;

  public:
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
};
