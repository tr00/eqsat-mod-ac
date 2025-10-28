#include <cassert>
#include <fstream>

#include "database.h"

void Database::clear_indices()
{
    indices.clear();
}

bool Database::rebuild(Handle handle)
{
    bool did_something = false;

    for (auto& [name, relation] : relations)
    {
        bool result = relation.rebuild(handle);
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
