#pragma once

#include <memory>
#include <vector>

#include "database.h"
#include "indices/abstract_index.h"
#include "query.h"

class State
{
    template <typename T>
    using Vec = std::vector<T>;
    using IndexPtr = std::shared_ptr<AbstractIndex>;
    using Iterator = gch::small_vector<id_t>::const_iterator;

  public:
    SortedVecSet candidates;
    Iterator candidate;

    Vec<IndexPtr> indices;

    void prepare();
    bool empty() const;
    id_t next();
    size_t intersect();
};

class Engine
{
  private:
    Vec<State> states;
    Database& db;

  public:
    Engine(Database& db) : db(db)
    {
    }

    // loads the required indices
    // and prepares the states
    void prepare(const Query& query);

    void execute();
};
