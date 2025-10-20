#pragma once

#include "../utils/multiset.h"

class MultisetSupport
{
  private:
    Multiset& mset;

  public:
    explicit MultisetSupport(Multiset& m)
        : mset(m)
    {
    }

    bool contains(id_t id) const
    {
        return mset.contains(id);
    }

    size_t size() const
    {
        return mset.size();
    }

    bool empty() const
    {
        return mset.empty();
    }

    template <typename Func>
    void for_each(Func f) const
    {
        for (const auto& [item, count] : mset.data)
            if (count > 0) f(item);
    }
};
