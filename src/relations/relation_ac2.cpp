#include <algorithm>

#include "relation_ac2.h"
#include "utils/multiset.h"

void RelationAC2::sort()
{
    std::sort(data.begin(), data.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;

        return lhs.second.hash() < rhs.second.hash();
    });
}

// O(mn)
bool RelationAC2::canonicalize(const Handle egraph)
{
    bool changed = false;

    for (auto& [id, mset] : data)
    {
        changed |= mset.map([egraph](id_t x) { return egraph.canonicalize(x); });

        id_t newid = egraph.canonicalize(id);
        if (id != newid)
        {
            changed = true;
            id = newid;
        }
    }

    // we mutated the tuples inplace
    // for future insertions/lookups they should be sorted.
    if (changed)
        sort();

    return changed;
}

namespace
{
using MultisetPtr = const Multiset *;

struct MultisetPtrHash
{
    size_t operator()(MultisetPtr ptr) const
    {
        return ptr->hash();
    }
};

struct MultisetPtrEqual
{
    bool operator()(MultisetPtr a, MultisetPtr b) const
    {
        return *a == *b;
    }
};
} // namespace

// O(mn)
// assumes canonical relation!
bool RelationAC2::congruence(Handle egraph)
{
    HashMap<MultisetPtr, id_t, MultisetPtrHash, MultisetPtrEqual> cache;

    bool changed = false;
    for (const auto& [id, mset] : data)
    {
        auto iter = cache.find(&mset);
        if (iter != cache.end()) // found!
        {
            id_t other_id = iter->second;
            if (egraph.equiv(id, other_id))
                continue;

            iter->second = egraph.unify(id, other_id);
            changed = true;
        }
        else
        {
            cache.emplace(&mset, id);
        }
    }

    return changed;
}

bool RelationAC2::flatten()
{
    for (const auto& [id_a, mset_a] : data)
    {
        for (const auto& [id_b, mset_b] : data)
        {
            if (mset_a.contains(id_b))
            {
            }
        }
    }
}

// O(lmn)
bool RelationAC2::rebuild(Handle egraph)
{
    bool changed = false;

    canonicalize(egraph);
    changed = congruence(egraph);

    while (changed)
    {
        changed = canonicalize(egraph);
        if (changed)
            changed = congruence(egraph);
    }

    // flatten
    // unflatten

    return true;
}

// how long can this keep going?
// well suppose we have a chain f(f(...f(a)))) and f(f(...f(b))) of length l
// and E contains only a=b. Well, then there will be l many iterations.
// which gives a total running time of O(lmn)
