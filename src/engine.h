#pragma once

#include <memory>
#include <vector>

#include "indices/abstract_index.h"

class State
{

    template <typename T> using Vec = std::vector<T>;
    using IndexPtr = std::shared_ptr<AbstractIndex>;
    using Iterator = gch::small_vector<id_t>::const_iterator;

  public:
    SortedVecSet candidates;
    Iterator candidate;

    const Vec<IndexPtr> indices;

    void prepare();
    bool empty() const;
    id_t next();
    size_t intersect();
};

class Engine
{
    std::vector<State> states;

    void run();
};
