#pragma once

#include <cstddef>

#include "types.h"

namespace eqsat
{

class SingletonSet
{
  private:
    id_t value;

  public:
    explicit SingletonSet(id_t id)
        : value(id)
    {
    }

    bool contains(id_t id) const
    {
        return value == id;
    }

    size_t size() const
    {
        return 1;
    }

    bool empty() const
    {
        return false;
    }

    id_t get() const
    {
        return value;
    }

    template <typename Func>
    void for_each(Func f) const
    {
        f(value);
    }
};

} // namespace eqsat
