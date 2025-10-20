#include "relation_ac.h"
#include "id.h"
#include "utils/multiset.h"
#include <functional>

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
    ankerl::unordered_dense::map<const Multiset *, id_t, MultisetPtrHash, MultisetPtrEqual> cache;

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
