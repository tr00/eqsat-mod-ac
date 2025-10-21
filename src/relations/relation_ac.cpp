#include <functional>

#include "gch/small_vector.hpp"
#include "id.h"
#include "relation_ac.h"
#include "utils/multiset.h"

namespace
{
struct MultisetPtrHash
{
    size_t operator()(const Multiset *ptr) const
    {
        return ptr->hash();
    }
};

struct MultisetPtrEqual
{
    bool operator()(const Multiset *a, const Multiset *b) const
    {
        return *a == *b;
    }
};
} // namespace

bool RelationAC::rebuild(std::function<id_t(id_t)> canonicalize, std::function<id_t(id_t, id_t)> unify)
{
    HashMap<const Multiset *, id_t, MultisetPtrHash, MultisetPtrEqual> cache;

    bool changed = false;

    for (auto& [eclass, map] : *data)
    {
        for (auto& [term, mset] : map)
        {
            // Canonicalize the multiset
            mset.map(canonicalize);

            // Check if we've seen this multiset before
            auto it = cache.find(&mset);
            if (it != cache.end())
            {
                // Found an equivalent multiset - unify their eclasses
                id_t other_eclass = it->second;
                if (eclass != other_eclass)
                {
                    unify(eclass, other_eclass);
                    changed = true;
                }
            }
            else
            {
                cache.emplace(&mset, eclass);
            }
        }
    }

    // TODO: remove duplicate entries after unification

    return changed;
}

void RelationAC::add_tuple(id_t id, Multiset mset)
{
    // is the new term included in any other existing term?
    Vec<Multiset> worklist;
    for (auto& [other_id, map] : *data)
    {
        worklist.clear();

        for (const auto& [_, other_mset] : map)
        {
            if (other_mset.includes(mset))
            {
                // Consider trying to add the term id1: { a, b }
                // while the term id0: { a, b, c, d } already exists.
                //
                // Then we create a new subterm id2: { id1, c, d }
                // which combines the id of the new term
                // with the multiset difference old \ new.

                auto diff = other_mset.msetdiff(mset);
                diff.insert(id);

                worklist.emplace_back(diff);
            }
        }

        // add subterms to term bank
        for (auto submset : worklist)
        {
            auto tid = static_cast<uint32_t>(nterms++);
            map.insert({tid, submset});
        }
    }

    // is any existing term included in the new term?
    for (auto& [other_id, map] : *data)
    {
        for (const auto& [_, other_mset] : map)
        {
            if (mset.includes(other_mset))
            {
            }
        }
    }

    auto tid = static_cast<uint32_t>(nterms++);
    if (!data->contains(id))
        data->emplace(id, HashMap<id_t, Multiset>{{tid, mset}});
    else
        (*data)[id].emplace(tid, mset);
}

void RelationAC::add_tuple(const Vec<id_t>& tuple)
{
    id_t eclass = tuple.back();

    // TODO: what about size 0, 1, 2, are they ub?
    Multiset mset{tuple.cbegin(), tuple.cend() - 1};

    add_tuple(eclass, mset);
}

//  -- Scenario 1 --
// term bank:
// x:{ a, b, c, d }
// y:{ a, b, c }
// x:{ y, d }
//
// insertion of z:{ a, b } will also produce
//
// x:{ z, c, d }
// y:{ z, c }

// -- Scenario 2 --
// term bank:
// x: { a, b, c }
// y: { a, b }
// x: { y, c }
//
// insertion of z: { a, b, c, d } will also produce
//
// z: { x, d }
// z: { y, c, d }
