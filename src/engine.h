#pragma once

#include <memory>

#include "database.h"
#include "indices/abstract_index.h"
#include "query.h"

class State
{
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

    Vec<id_t> execute();
};
