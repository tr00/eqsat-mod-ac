#include <cassert>
#include <fstream>
#include <functional>

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

bool Database::rebuild(std::function<id_t(id_t)> canonicalize, std::function<id_t(id_t, id_t)> unify)
{
    bool did_something = false;

    for (auto& [name, relation] : relations)
    {
        bool result = relation.rebuild(canonicalize, unify);
        did_something = did_something || result;
    }

    return did_something;
}

void Database::dump_to_file(const std::string& filename, const SymbolTable& symbols) const
{
    std::ofstream out(filename);
    if (!out.is_open())
    {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    out << "========================================" << std::endl;
    out << "DATABASE DUMP" << std::endl;
    out << "========================================" << std::endl;
    out << std::endl;

    for (const auto& [name, relation] : relations)
    {
        relation.dump(out, symbols);
    }

    out.close();
}
