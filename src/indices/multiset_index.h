#pragma once

#include "../sets/abstract_set.h"
#include "../utils/multiset.h"
#include "id.h"

class MultisetIndex
{
  private:
    HashMap<id_t, Multiset> rel;
    Multiset *mset;
    Vec<id_t> history;

  public:
    MultisetIndex(const HashMap<id_t, Multiset> rel) : rel(rel)
    {
    }

    AbstractSet project();
    void select(id_t key);
    void unselect();
    void reset();
};
