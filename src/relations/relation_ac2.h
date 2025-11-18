#pragma once

#include <utility>

#include "handle.h"
#include "utils/multiset.h"
#include "utils/vec.h"

class RelationAC2
{
  private:
    Vec<std::pair<id_t, Multiset>> data;

    void sort();
    bool canonicalize(const Handle egraph);
    bool congruence(Handle egraph);
    bool flatten();

  public:
    // add_tuple

    bool rebuild(Handle egraph);
};
