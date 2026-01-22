#include <functional>

#include "engine.h"
#include "query.h"
#include "sets/abstract_set.h"

namespace eqsat
{

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

id_t State::current() const
{
    return *(it - 1);
}

size_t Engine::intersect(State& state)
{
    Vec<AbstractSet> sets;

    if (state.fd != nullptr)
    {
        auto enode = state.fd->make_enode();
        auto id = lookup_or_ephemeral(enode);

        sets.emplace_back(AbstractSet(SingletonSet(id)));
    }

    for (const auto& index : state.indices)
        sets.push_back(index->project());

    return intersect_many(state.candidates, sets);
}

void Engine::prepare(const Query& query)
{
    using ConstraintRef = std::reference_wrapper<const Constraint>;

    HashMap<Constraint, std::shared_ptr<AbstractIndex>> indices;
    HashMap<var_t, Vec<ConstraintRef>> constraints;

    // Reset ephemeral state
    // ephemeral_counter = 0;
    // ephemeral_map.clear();

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

            if (var == constraint.variables.back() && constraint.permutation == static_cast<uint32_t>(AC))
            {
                state.fd = index_it->second;
            }
            else
            {
                state.indices.push_back(index_it->second);
            }
        }
        states.push_back(std::move(state));
    }

    // Reset all indices to root before execution
    for (auto& [constraint, index] : indices)
        index->reset();
}

void Engine::execute(Vec<id_t>& results, const Query& query)
{
    prepare(query);

    execute_rec(results, 0);
}

void Engine::execute_rec(Vec<id_t>& results, size_t level)
{
    if (level >= states.size())
    {
        for (var_t var : head)
            results.push_back(states[var].current());

        return;
    }

    auto& state = states[level];

    // projection & intersection
    intersect(state);

    state.prepare();
    while (!state.empty())
    {
        id_t cand = state.next();

        for (auto index : state.indices)
            index->select(cand);

        execute_rec(results, level + 1);

        for (auto index : state.indices)
            index->unselect();
    }

    return;
}

} // namespace eqsat
