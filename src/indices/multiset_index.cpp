#include "multiset_index.h"
#include "enode.h"
#include "id.h"
#include "sets/abstract_set.h"
#include "sets/hashmap_wrapper.h"
#include "utils/multiset.h"

AbstractSet MultisetIndex::project()
{
    if (!mset.has_value()) // term-id
    {
        return AbstractSet(WrappedHashMapSet(data));
    }
    else if (!mset.value()->empty()) // children...
    {
        const Multiset& cref = *mset.value();
        return AbstractSet(MultisetSupport(cref));
    }

    return AbstractSet();
}

void MultisetIndex::select(id_t key)
{
    if (!mset.has_value()) // term-id
    {
        mset = &data.at(key);
        return;
    }

    auto& args = mset.value();

    if (args->empty())
        return;

    history.push_back(key);

    // We search in the multiset for this key and decrement its counter.
    // This means we have temporarily removed it from the set.
    // In order to later "unselect" this key, we've added it to the history.
    args->remove(key);
}

void MultisetIndex::unselect()
{
    if (history.empty()) // term-id
    {
        mset = std::nullopt;
    }
    else // children...
    {
        auto key = history.back();
        history.pop_back();
        mset.value()->insert(key);
    }
}

ENode MultisetIndex::make_enode()
{
    return ENode(symbol, history);
}

void MultisetIndex::reset()
{
    // Restore all removed elements
    while (!history.empty())
    {
        auto key = history.back();
        history.pop_back();
        mset.value()->insert(key);
    }

    mset = std::nullopt;
}
