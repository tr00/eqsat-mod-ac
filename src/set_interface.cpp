#include <algorithm>

#include "set_interface.h"
#include "sorted_set.h"

SetInterface intersect(const std::vector<std::reference_wrapper<const SetInterface>>& sets) {
    if (sets.empty()) {
        return SetInterface(SortedSet{});
    }

    if (sets.size() == 1) {
        // Create a copy of the single set using copy_into
        SortedSet result = sets[0].get().copy_into<SortedSet>();
        return SetInterface(std::move(result));
    }

    // Find the smallest set to start with (optimization)
    auto smallest_it = std::min_element(sets.begin(), sets.end(),
        [](const auto& a, const auto& b) {
            return a.get().size() < b.get().size();
        });

    SortedSet result;

    // Iterate through the smallest set and check if each element exists in all other sets
    smallest_it->get().for_each([&](id_t id) {
        bool in_all_sets = true;
        for (const auto& set_ref : sets) {
            if (!set_ref.get().contains(id)) {
                in_all_sets = false;
                break;
            }
        }
        if (in_all_sets) {
            result.insert(id);
        }
    });

    return SetInterface(std::move(result));
}
