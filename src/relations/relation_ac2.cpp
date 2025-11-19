#include <algorithm>

#include "indices/abstract_index.h"
#include "indices/multiset_index.h"
#include "relation_ac2.h"
#include "utils/hashmap.h"
#include "utils/multiset.h"

bool RelationAC2::insert(std::pair<id_t, Multiset> tuple)
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

void RelationAC2::sort()
{
    std::sort(data.begin(), data.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;

        return lhs.second.hash() < rhs.second.hash();
    });
}

void RelationAC2::add_tuple(id_t id, Multiset mset)
{
    insert({id, mset});
}

AbstractIndex RelationAC2::populate_index()
{
    HashMap<id_t, Multiset> index;

    size_t n = data.size();
    for (size_t i = 0; i < n; ++i)
    {
        index.insert({/* term-id: */ i, data[i].second});
    }

    return AbstractIndex(MultisetIndex(symbol, index));
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

    // we mutated the tuples inplace so they are no longer ordered.
    // For future insertions/lookups they should be sorted.
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
    bool changed = false;

    for (const auto& [id_a, mset_a] : data)
    {
        for (const auto& [id_b, mset_b] : data)
        {
            if (mset_a.contains(id_b))
            {
                // a = f(X \cup {b})
                // b = f(Y)
                // ~~> a = f(X \cup Y)
                auto args = mset_a;
                args.remove(id_b);
                args.insert_all(mset_b);

                insert({id_a, args});

                changed = true;
            }
        }
    }

    return changed;
}

bool RelationAC2::unflatten()
{
    bool changed = false;

    for (const auto& [id_a, mset_a] : data)
    {
        for (const auto& [id_b, mset_b] : data)
        {
            if (mset_a.includes(mset_b))
            {
                // a = f(X \cup Y)
                // b = f(Y)
                // ~~> a = f(X \cup {b})
                auto args = mset_a.msetdiff(mset_b);
                args.insert(id_b);

                // TODO: assert size > 1 (?)

                insert({id_a, args});

                changed = true;
            }
        }
    }

    return changed;
}

// how long can this keep going?
// well suppose we have a chain f(f(...f(a)))) and f(f(...f(b))) of length l
// and E contains only a=b. Well, then there will be l many iterations.
// which gives a total running time of O(lmn)

void RelationAC2::dump(std::ofstream& out, const SymbolTable& symbols) const
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
bool RelationAC2::rebuild(Handle egraph)
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

    flatten();
    unflatten();

    return true;
}
