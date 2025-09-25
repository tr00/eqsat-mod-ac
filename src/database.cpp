#include "database.h"
#include <cassert>

void Database::build_indices() {
    for (auto& [index_key, index] : indices) {
        auto [rel_name, perm] = index_key;
        auto index_ref = index;

        auto relation = get_relation(rel_name);
        assert(relation != nullptr);

        relation->build_index(perm);
    }
}

void Database::clear_indices() {
    indices.clear();
}
