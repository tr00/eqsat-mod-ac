#pragma once

#include <algorithm>

#include "types.h"

namespace eqsat
{

class SortedIterSet
{
  private:
    Vec<id_t>::const_iterator begin;
    Vec<id_t>::const_iterator end;

  public:
    SortedIterSet(const Vec<id_t>& data)
        : begin(data.begin())
        , end(data.end())
    {
    }

    bool contains(id_t id) const
    {
        auto it = std::lower_bound(begin, end, id);
        return it != end && *it == id;
    }

    size_t size() const
    {
        return end - begin;
    }

    template <typename Func>
    void for_each(Func f) const
    {
        for (auto it = begin; it != end; ++it)
            f(*it);
    }
};

} // namespace eqsat
