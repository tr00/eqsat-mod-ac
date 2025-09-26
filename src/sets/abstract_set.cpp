#include <algorithm>

#include "abstract_set.h"
#include "sorted_set.h"

// TODO: partially sorting.

void intersect_many(SortedVecSet &output, const std::vector<AbstractSet> &sets)
{
    output.clear();

    if (sets.empty())
    {
        return;
    }

    if (sets.size() == 1)
    {
        // Copy the single set into the output buffer
        sets[0].for_each([&output](id_t id) { output.insert(id); });
        return;
    }

    // Find the smallest set to start with (optimization)
    auto smallest_it =
        std::min_element(sets.begin(), sets.end(), [](const auto &a, const auto &b) { return a.size() < b.size(); });

    // Iterate through the smallest set and check if each element exists in all other sets
    smallest_it->for_each([&](id_t id) {
        bool in_all_sets = true;
        for (const auto &set : sets)
        {
            if (!set.contains(id))
            {
                in_all_sets = false;
                break;
            }
        }
        if (in_all_sets)
        {
            output.insert(id);
        }
    });
}
