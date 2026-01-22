#pragma once

#include <memory>

#include "database.h"
#include "egraph_di.h"
#include "indices/abstract_index.h"
#include "query.h"

namespace eqsat
{

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
    // lookup on this index rather than project/select.
    // To perform lookup we reuse the egraphs memo table.
    //
    // Note that with the current API of compiling pattern expressions,
    // at most one FD can be inferred per variable.
    std::shared_ptr<AbstractIndex> fd = nullptr;

    void prepare();
    bool empty() const;
    id_t next();
    id_t current() const;
};

class Engine : EGraphLookupDI
{
  private:
    Vec<State, 0> states;
    Vec<var_t> head;
    const Database& db;

  public:
    Engine(const Database& db, EGraph& egraph)
        : EGraphLookupDI(egraph)
        , db(db)
    {
    }

    // loads the required indices
    // and prepares the states
    void prepare(const Query& query);

    size_t intersect(State& state);

    void execute(Vec<id_t>& buffer, const Query& query);
    void execute_rec(Vec<id_t>& results, size_t level);
};

} // namespace eqsat
