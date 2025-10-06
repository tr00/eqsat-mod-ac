#include "multiset_index.h"
#include "sets/abstract_set.h"

AbstractSet MultisetIndex::project()
{
    return AbstractSet(MultisetSupport(*this->mset));
}

void MultisetIndex::select(id_t key)
{
    if (mset == nullptr)
    {
        mset = &rel[key];
    }
    else
    {
        history.push_back(key);

        // We search in the multiset for this key and decrement its counter.
        // This means we have temporarily removed it from the set.
        // In order to later "unselect" this key, we've added it to the history.
        mset->remove(key);
    }
}

void MultisetIndex::unselect()
{
    if (history.empty())
    {
        mset = nullptr;
    }
    else
    {
        auto key = history.back();
        history.pop_back();
        mset->insert(key);
    }
}

void MultisetIndex::reset()
{
    // Restore all removed elements
    while (!history.empty())
    {
        auto key = history.back();
        history.pop_back();
        mset->insert(key);
    }
    mset = nullptr;
}
