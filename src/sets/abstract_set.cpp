#include <algorithm>

#include "abstract_set.h"

namespace eqsat
{

// TODO: partially sorting.

size_t intersect_many(SortedVecSet& output, const Vec<AbstractSet>& sets)
{
    output.clear();

    if (sets.empty())
    {
        return 0;
    }

    if (sets.size() == 1)
    {
        sets[0].for_each([&output](id_t id) { output.insert(id); });
        return output.size();
    }

    auto cmp = [](const auto& a, const auto& b) { return a.size() < b.size(); };
    auto smallest_it = std::min_element(sets.begin(), sets.end(), cmp);

    smallest_it->for_each([&](id_t id) {
        bool in_all_sets = true;
        for (const auto& set : sets)
        {
            if (!set.contains(id))
            {
                in_all_sets = false;
                break;
            }
        }
        if (in_all_sets)
            output.insert(id);
    });

    return output.size();
}

} // namespace eqsat
