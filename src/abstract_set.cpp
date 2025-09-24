#include <algorithm>

#include "abstract_set.h"
#include "sorted_set.h"

AbstractSet intersect(const std::vector<std::reference_wrapper<const AbstractSet>>& sets) {
    if (sets.empty()) {
        return AbstractSet(SortedSet{});
    }

    if (sets.size() == 1) {
        // Create a copy of the single set using copy_into
        SortedSet result = sets[0].get().copy_into<SortedSet>();
        return AbstractSet(std::move(result));
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

    return AbstractSet(std::move(result));
}

void intersect(SortedSet& output, const std::vector<std::reference_wrapper<const AbstractSet>>& sets) {
    output.clear();

    if (sets.empty()) {
        return;
    }

    if (sets.size() == 1) {
        // Copy the single set into the output buffer
        sets[0].get().for_each([&output](id_t id) {
            output.insert(id);
        });
        return;
    }

    // Find the smallest set to start with (optimization)
    auto smallest_it = std::min_element(sets.begin(), sets.end(),
        [](const auto& a, const auto& b) {
            return a.get().size() < b.get().size();
        });

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
            output.insert(id);
        }
    });
}
