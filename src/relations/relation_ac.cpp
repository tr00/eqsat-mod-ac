#include <algorithm>
#include <utility>

#include "enode.h"
#include "indices/abstract_index.h"
#include "indices/multiset_index.h"
#include "relation_ac.h"
#include "utils/hashmap.h"
#include "utils/multiset.h"

namespace eqsat
{

bool RelationAC::insert(std::pair<id_t, Multiset> tuple)
{
    auto cmp = [](const std::pair<id_t, Multiset>& lhs, const std::pair<id_t, Multiset>& rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;
        return lhs.second.hash() < rhs.second.hash();
    };

    auto it = std::lower_bound(data.begin(), data.end(), tuple, cmp);

    // TODO:
    // Note that this is a silent bug waiting to happen...
    // we assume that the hash is enough to differentiate multisets
    // and don't perform any collision detection!
    if (it != data.end() && it->first == tuple.first && it->second.hash() == tuple.second.hash())
    {
        return false;
    }

    data.insert(it, std::move(tuple));
    return true;
}

bool RelationAC::contains(std::pair<id_t, Multiset> tuple)
{
    auto cmp = [](const std::pair<id_t, Multiset>& lhs, const std::pair<id_t, Multiset>& rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;
        return lhs.second.hash() < rhs.second.hash();
    };

    auto it = std::lower_bound(data.begin(), data.end(), tuple, cmp);

    return it != data.end() && it->first == tuple.first && it->second.hash() == tuple.second.hash();
}

void RelationAC::sort()
{
    std::sort(data.begin(), data.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;

        return lhs.second.hash() < rhs.second.hash();
    });
}

void RelationAC::add_tuple(id_t id, Multiset mset)
{
    insert({id, mset});
}

void RelationAC::add_tuple(const Vec<id_t>& tuple)
{
    id_t id = tuple.back();
    Multiset mset{tuple.cbegin(), tuple.cend() - 1};

    insert({id, mset});
}

AbstractIndex RelationAC::populate_index(uint32_t)
{
    HashMap<id_t, Multiset> index;

    size_t n = data.size();
    for (size_t i = 0; i < n; ++i)
    {
        index.insert({/* term-id: */ i, data[i].second});
    }

    return AbstractIndex(MultisetIndex(symbol, index));
}

void RelationAC::deduplicate()
{
    if (data.empty())
        return;

    size_t write_idx = 0;
    for (size_t read_idx = 1; read_idx < data.size(); ++read_idx)
    {
        if (data[write_idx].first != data[read_idx].first ||
            data[write_idx].second.hash() != data[read_idx].second.hash())
        {
            ++write_idx;
            if (write_idx != read_idx)
            {
                data[write_idx] = std::move(data[read_idx]);
            }
        }
    }

    data.resize(write_idx + 1);
}

// O(mn)
bool RelationAC::canonicalize(const Handle egraph)
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

    // we mutated the tuples inplace so they are no longer ordered.
    // For future insertions/lookups they should be sorted.
    if (changed)
    {
        sort();
        deduplicate();
    }

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
bool RelationAC::congruence(Handle egraph)
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

bool RelationAC::flatten(Handle egraph)
{
    bool changed = false;

    Vec<std::pair<id_t, Multiset>> worklist;
    for (const auto& [id_a, mset_a] : data)
    {
        for (const auto& [id_b, mset_b] : data)
        {
            if (mset_a.hash() == mset_b.hash())
                continue;

            if (mset_b.contains(id_b)) // cyclic
                continue;

            if (!mset_a.contains(id_b))
                continue;

            // a = f(X \cup {b})
            // b = f(Y)
            // ~~> a = f(X \cup Y)
            auto args = mset_a;
            args.remove(id_b);
            args.insert_all(mset_b);

            if (contains({id_a, args}))
                continue;

            worklist.push_back({id_a, args});
            changed = true;
        }
    }

    for (const auto& [id, mset] : worklist)
    {
        ENode enode{symbol, mset.collect()};
        egraph.add_enode_to_memo(id, enode);
        insert({id, mset});
    }

    return changed;
}

bool RelationAC::unflatten(Handle egraph)
{
    bool changed = false;

    Vec<std::pair<id_t, Multiset>> worklist;

    for (const auto& [id_a, mset_a] : data)
    {
        for (const auto& [id_b, mset_b] : data)
        {
            if (mset_a.hash() == mset_b.hash())
                continue;

            if (!mset_a.includes(mset_b))
                continue;

            // a = f(X \cup Y)
            // b = f(Y)
            // ~~> a = f(X \cup {b})
            auto args = mset_a.msetdiff(mset_b);
            args.insert(id_b);

            if (contains({id_a, args}))
                continue;

            // TODO: assert size > 1 (?)
            worklist.push_back({id_a, args});
            changed = true;
        }
    }

    for (const auto& [id, mset] : worklist)
    {
        ENode enode{symbol, mset.collect()};
        egraph.add_enode_to_memo(id, enode);
        insert({id, mset});
    }

    return changed;
}

bool RelationAC::rebuild(Handle egraph)
{
    bool changed = false;

    canonicalize(egraph);
    changed = congruence(egraph);

    for (size_t i = 30; changed && i >= 0; --i)
    {
        changed = canonicalize(egraph);
        if (changed)
            changed = congruence(egraph);
    }

    flatten(egraph);
    unflatten(egraph);

    return true;
}

void RelationAC::dump(std::ofstream& out, const SymbolTable& symbols) const
{
    out << "---- " << symbols.get_string(symbol) << "(AC) with " << size() << " tuples ----" << std::endl;

    for (const auto& [eclass_id, mset] : data)
    {
        out << "eclass-id: " << eclass_id << "  mset: {{";

        bool first = true;
        for (const auto& [id, count] : mset.data)
        {
            if (!first)
                out << ", ";
            first = false;

            out << id;
            if (count > 1)
                out << "^" << count;
        }

        out << "}}" << std::endl;
    }
    out << std::endl;
}

} // namespace eqsat
