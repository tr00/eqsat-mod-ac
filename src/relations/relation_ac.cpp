#include <functional>
#include <iostream>

#include "enode.h"
#include "gch/small_vector.hpp"
#include "handle.h"
#include "id.h"
#include "relation_ac.h"
#include "utils/hashmap.h"
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
    Vec<id_t> terms_to_keep;

    // canonicalize all elements in the multisets and detect duplicates
    for (auto& [term, mset] : *data)
    {
        mset.map([egraph](id_t x) { return egraph.canonicalize(x); });

        // check if we've seen this multiset before
        // because if so, then they are equal via congruence
        id_t id = ids[term];
        auto it = cache.find(&mset);
        if (it != cache.end())
        {
            // Duplicate found - unify and skip this term
            id_t other_id = it->second;
            if (id != other_id) egraph.unify(id, other_id);
            // Don't add to terms_to_keep
        }
        else
        {
            // First occurrence - keep this term
            cache.emplace(&mset, id);
            terms_to_keep.push_back(term);
        }
    }

    // Rebuild with only unique terms and canonicalized eclass-ids
    auto new_data = std::make_shared<HashMap<id_t, Multiset>>();
    Vec<id_t> new_ids;

    for (id_t old_term : terms_to_keep)
    {
        id_t new_term = static_cast<id_t>(new_ids.size());
        id_t eclass_id = egraph.canonicalize(ids[old_term]);

        new_data->emplace(new_term, std::move((*data)[old_term]));
        new_ids.push_back(eclass_id);
    }

    // Replace old structures with compacted ones
    data = new_data;
    ids = std::move(new_ids);

    return true;
}

void RelationAC::add_tuple(id_t id, Multiset mset)
{
    Vec<std::pair<id_t, Multiset>> worklist;

    // is the new term included in any other existing term?
    for (auto& [other_term, other_mset] : *data)
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
            if (diff.empty()) continue;
            diff.insert(id);
            auto other_id = ids[other_term];

            worklist.push_back({other_id, std::move(diff)});
        }
    }

    // is any existing term included in the new term?
    for (auto& [other_term, other_mset] : *data)
    {
        if (mset.includes(other_mset))
        {
            auto other_id = ids[other_term];
            auto diff = mset.msetdiff(other_mset);
            if (diff.empty()) continue;
            diff.insert(other_id);

            worklist.push_back({id, std::move(diff)});
        }
    }

    // add subterms to term bank
    for (const auto& [eclass, submset] : worklist)
    {
        auto term_id = static_cast<uint32_t>(size());
        ids.push_back(eclass);
        data->emplace(term_id, submset);

        ENode enode{symbol, submset.collect()};
        egraph.add_enode_to_memo(eclass, enode);
    }

    auto term_id = static_cast<uint32_t>(size());
    ids.push_back(id);
    data->insert({term_id, mset});
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

    for (const auto& [term, mset] : *data)
    {
        auto eclass = ids[term];

        out << "eclass-id: " << eclass << "  term-id:" << term << "  mset: {";

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
    out << std::endl;
}
