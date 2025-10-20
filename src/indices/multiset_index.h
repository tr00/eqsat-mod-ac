#pragma once

#include <optional>

#include "../sets/abstract_set.h"
#include "../utils/multiset.h"
#include "id.h"

class MultisetIndex
{
  private:
    // eclass-id < term-id < args...
    std::shared_ptr<const HashMap<id_t, HashMap<id_t, Multiset>>> data;
    std::optional<Multiset> mset;
    Vec<id_t> history;

  public:
    MultisetIndex(std::shared_ptr<const HashMap<id_t, HashMap<id_t, Multiset>>> data)
        : data(data)
        , history()
        , mset()
    {
    }

    AbstractSet project();
    void select(id_t key);
    void unselect();
    void reset();
};
