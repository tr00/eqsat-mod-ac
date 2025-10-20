#include "multiset_index.h"
#include "id.h"
#include "sets/abstract_set.h"
#include "sets/hashmap_wrapper.h"
#include "utils/multiset.h"
#include <optional>

AbstractSet MultisetIndex::project()
{
    switch (history.size())
    {
    case 0: // eclass-id
        return AbstractSet(WrappedHashMapSet(*data));
    case 1: // term-id
    {
        auto cid = history[0];
        return AbstractSet(WrappedHashMapSet(data->at(cid)));
    }
    default: // args...
        return AbstractSet(MultisetSupport(*mset));
    }

    assert(0);
}

void MultisetIndex::select(id_t key)
{
    // eclass-id < term-id < args...
    switch (history.size())
    {
    case 0:
        history.push_back(key);
        break;
    case 1:
        history.push_back(key);
        mset = data->at(history[0]).at(key);
        break;
    default:
        history.push_back(key);
        // We search in the multiset for this key and decrement its counter.
        // This means we have temporarily removed it from the set.
        // In order to later "unselect" this key, we've added it to the history.
        mset->remove(key);
        break;
    }
}

void MultisetIndex::unselect()
{
    switch (history.size())
    {
    case 0:
        break;
    case 1: // eclass-id
        history.pop_back();
        break;
    case 2: // term-id
        history.pop_back();
        mset = std::nullopt;
        break;
    default: // args...
    {
        auto key = history.back();
        history.pop_back();
        mset->insert(key);
    }
    }
}

void MultisetIndex::reset()
{
    // Restore all removed elements
    while (history.size() > 2)
    {
        auto key = history.back();
        history.pop_back();
        mset->insert(key);
    }
    history.clear();
    mset = std::nullopt;
}
