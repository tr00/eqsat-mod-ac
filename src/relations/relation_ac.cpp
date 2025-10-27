#include <functional>

#include "gch/small_vector.hpp"
#include "handle.h"
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

bool RelationAC::rebuild(Handle egraph)
{
    HashMap<const Multiset *, id_t, MultisetPtrHash, MultisetPtrEqual> cache;

    for (auto& [eclass, map] : *data)
    {
        for (auto& [term, mset] : map)
        {
            // update all multisets
            mset.map([egraph](id_t x) { return egraph.canonicalize(x); });

            // check if we've seen this multiset before
            // because if so then they are eq via congruence
            auto it = cache.find(&mset);
            if (it != cache.end())
            {
                id_t other_eclass = it->second;
                if (eclass != other_eclass) egraph.unify(eclass, other_eclass);
            }
            else
            {
                cache.emplace(&mset, eclass);
            }
        }
    }

    // the keys might have also changed
    Vec<id_t> changed_keys;
    for (const auto& [id, _] : *data)
        if (egraph.canonicalize(id) != id) changed_keys.push_back(id);

    for (auto oldkey : changed_keys)
    {
        auto newkey = egraph.canonicalize(oldkey);

        if (!data->contains(newkey))
        {
            auto kv = data->extract(oldkey);
            assert(kv.has_value());
            kv->first = newkey;
            data->insert(std::move(kv.value()));
        }
        else
        {
            // if newkey already has a few terms
            // we append the terms from the old map

            auto& oldmap = data->at(oldkey);
            auto& newmap = data->at(newkey);

            newmap.reserve(oldmap.size());

            for (auto kv : oldmap)
                newmap.insert(kv);

            data->erase(oldkey);
        }
    }

    // TODO: remove duplicate entries after unification

    return true;
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

                worklist.push_back(std::move(diff));
            }
        }

        // add subterms to term bank
        for (const auto& submset : worklist)
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
                auto diff = mset.msetdiff(other_mset);
                diff.insert(other_id);

                worklist.push_back(std::move(diff));
            }
        }
    }

    auto tid = static_cast<uint32_t>(nterms++);
    if (!data->contains(id)) data->emplace(id, HashMap<id_t, Multiset>{});

    auto& map = data->at(id);
    for (const auto& submset : worklist)
    {
        auto tid = static_cast<uint32_t>(nterms++);
        map.insert({tid, submset});
    }

    map.emplace(tid, mset);
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

void RelationAC::dump(std::ofstream& out, const SymbolTable& symbols) const
{
    out << "---- " << symbols.get_string(symbol) << "(AC) with " << size() << " terms ----" << std::endl;

    for (const auto& [eclass, map] : *data)
    {
        for (const auto& [term_id, mset] : map)
        {
            out << "eclass-id: " << eclass << "  term-id:" << term_id << "  mset: {";

            bool first = true;
            for (const auto& [id, count] : mset.data)
            {
                if (!first) out << ", ";
                first = false;

                out << id;
                if (count > 1) out << "^" << count;
            }

            out << "}" << std::endl;
        }
    }
    out << std::endl;
}
