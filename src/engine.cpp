#include "engine.h"
#include "sets/abstract_set.h"
#include <functional>
#include <vector>

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

void State::prepare() {
    candidate = candidates.begin();
}

bool State::empty() const {
    return candidate != candidates.end();
}

id_t State::next() {
    return *candidate++;
}

size_t State::intersect() {
    std::vector<AbstractSet> buffer;

    for (auto index : indices)
        buffer.push_back(index->project());

    intersect_many(candidates, buffer);
    return candidates.size();
}

void Engine::run() {
    std::vector<id_t> results;
    auto state = states.begin();

    id_t cand;

DEEPER:
    if (state == states.end())
        goto YIELD;

    if(state->intersect() == 0)
        goto BACKTRACK;

    state->prepare();

    // first candidate
    cand = state->next();

    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

BACKTRACK:

    --state;

    for (auto index : state->indices)
        index->backtrack();

    if (state->empty())
        goto BACKTRACK;

    // ith candidate
    cand = state->next();
    for (auto index : state->indices)
        index->select(cand);

    ++state;

    goto DEEPER;

YIELD:

    for (auto state : states) {
        results.push_back(*state.candidate);
    }

}
