#include <functional>

#include "engine.h"
#include "id.h"
#include "query.h"
#include "sets/abstract_set.h"

//                            ▲
// descend into   │           │ ascend out / backtrack
//                │           │
//                ▼
//               ┌─────────────┐
//               │             │
//               │    State    │
//               │             │
//               └─────────────┘
//                            ▲
//                │           │ ascend into
//  descend out   │           │
//                ▼

void State::prepare()
{
    it = candidates.begin();
}

bool State::empty() const
{
    return it == candidates.end();
}

id_t State::next()
{
    return *it++;
}

size_t Engine::intersect(State& state)
{
    if (state.fds.empty())
    {
        Vec<AbstractSet> buffer;

        for (const auto& index : state.indices)
            buffer.push_back(index->project());

        return intersect_many(state.candidates, buffer);
    }
    else
    {
        for (auto& index : state.fds)
        {
            auto enode = index->make_enode();
            // TODO: ephemeral enodes
            egraph.lookup(enode);
        }
    }
}

void Engine::prepare(const Query& query)
{
    using ConstraintRef = std::reference_wrapper<const Constraint>;

    HashMap<Constraint, std::shared_ptr<AbstractIndex>> indices;
    HashMap<var_t, Vec<ConstraintRef>> constraints;

    // load indices
    for (const auto& constraint : query.constraints)
    {
        uint32_t permutation = constraint.permutation;
        auto index = db.get_index(constraint.symbol, permutation);

        indices[constraint] = std::make_shared<AbstractIndex>(index);

        for (var_t var : constraint.variables)
        {
            auto it = constraints.find(var);
            if (it != constraints.end())
                it->second.push_back(std::cref(constraint));
            else
                constraints[var] = Vec<ConstraintRef>{std::cref(constraint)};
        }
    }

    // store head variables
    head = query.head;

    // initialize states
    states.clear();
    for (var_t var = 0; var < query.nvars; ++var)
    {
        auto it = constraints.find(var);
        assert(it != constraints.end());
        const auto& var_constraints = it->second;

        State state;
        for (const auto& constraint_ref : var_constraints)
        {
            const Constraint& constraint = constraint_ref.get();
            auto index_it = indices.find(constraint);
            assert(index_it != indices.end());
            state.indices.push_back(index_it->second); // Shares the index via shared_ptr
        }
        states.push_back(std::move(state));
    }

    // Reset all indices to root before execution
    for (auto& [constraint, index] : indices)
        index->reset();
}

void Engine::execute_rec(Vec<id_t>& results, size_t level)
{
    if (level >= states.size())
    {
        for (var_t var : head)
            results.push_back(var);

        return;
    }

    auto& state = states[level];

    // projection & intersection
    intersect(state);

    for (auto cand : state.candidates)
    {
        for (auto index : state.indices)
            index->select(cand);

        execute_rec(results, level + 1);

        for (auto index : state.indices)
            index->unselect();
    }

    return;
}

Vec<id_t> Engine::execute()
{
    Vec<id_t> results;
    auto state = states.begin();

    id_t cand;

DEEPER:
    if (state == states.end()) goto YIELD;

    if (intersect(*state) == 0) goto BACKTRACK;

    state->prepare();

    // first candidate
    cand = state->next();

    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

BACKTRACK:

    if (state == states.begin()) return results;

    --state;

    for (auto index : state->indices)
        index->unselect();

    if (state->empty()) goto BACKTRACK;

    // ith candidate
    cand = state->next();
    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

YIELD:

    for (var_t var : head)
        results.push_back(*(states[var].it - 1));

    goto BACKTRACK;
}
