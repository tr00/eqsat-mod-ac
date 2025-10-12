#include <cassert>

#include "database.h"

void Database::populate_indices()
{
    for (auto& [index_key, index] : indices)
    {
        auto [rel_name, perm] = index_key;

        auto relation = get_relation(rel_name);
        assert(relation != nullptr);

        index = relation->populate_index(perm);
    }
}

void Database::clear_indices()
{
    indices.clear();
}
