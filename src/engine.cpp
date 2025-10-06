#include <functional>
#include <memory>

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
    candidate = candidates.begin();
}

bool State::empty() const
{
    return candidate == candidates.end();
}

id_t State::next()
{
    return *candidate++;
}

size_t State::intersect()
{
    Vec<AbstractSet> buffer;

    for (auto index : indices)
        buffer.push_back(index->project());

    intersect_many(candidates, buffer);
    return candidates.size();
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
        auto index = db.get_index(constraint.operator_symbol, permutation);

        indices[constraint] = index;

        for (var_t var : constraint.variables)
        {
            auto it = constraints.find(var);
            if (it != constraints.end())
                it->second.push_back(std::cref(constraint));
            else
                constraints[var] = Vec<ConstraintRef>{std::cref(constraint)};
        }
    }

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
            state.indices.push_back(index_it->second);
        }
        states.push_back(std::move(state));
    }

    // Reset all indices to root before execution
    for (auto& [constraint, index] : indices)
    {
        index->reset();
    }
}

Vec<id_t> Engine::execute()
{
    Vec<id_t> results;
    auto state = states.begin();

    id_t cand;

DEEPER:
    if (state == states.end())
        goto YIELD;

    if (state->intersect() == 0)
        goto BACKTRACK;

    state->prepare();

    // first candidate
    cand = state->next();

    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

BACKTRACK:

    if (state == states.begin())
        return results;

    --state;

    for (auto index : state->indices)
        index->unselect();

    if (state->empty())
        goto BACKTRACK;

    // ith candidate
    cand = state->next();
    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

YIELD:

    for (auto state : states)
    {
        results.push_back(*state.candidate);
    }

    goto BACKTRACK;
}
