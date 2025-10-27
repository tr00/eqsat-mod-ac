#pragma once

#include <memory>

#include "database.h"
#include "indices/abstract_index.h"
#include "query.h"

struct State
{
    Vec<id_t>::const_iterator it;
    SortedVecSet candidates;

    Vec<std::shared_ptr<AbstractIndex>> indices;

    // There is a function dependency on the ids of each constraint.
    // For example add(x, y; id) has the FD {x,y} --> id
    // meaning there can only be one term 'x + y' and not multiple.
    // The id is then uniquely determined by memo[(x, y)].
    //
    // If this state corresponds to id, we can call
    // lookup on the indices rather than select.
    // To perform lookup we reuse the egraphs memo table.
    Vec<std::shared_ptr<AbstractIndex>> fds;

    void prepare();
    bool empty() const;
    id_t next();
};

class Engine
{
  private:
    Vec<State, 0> states;
    Vec<var_t> head;
    const Database& db;
    const Handle egraph;

  public:
    Engine(const Database& db, const Handle handle)
        : db(db)
        , egraph(handle)
    {
    }

    // loads the required indices
    // and prepares the states
    void prepare(const Query& query);

    size_t intersect(State& state);

    Vec<id_t> execute();
    void execute_rec(Vec<id_t>& results, size_t level);
};
