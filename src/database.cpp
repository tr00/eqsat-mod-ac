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

void Database::dump_to_file(std::ofstream& out, const SymbolTable& symbols) const
{

    out << "====<< Database >>====\n\n";

    for (const auto& [name, relation] : relations)
        relation.dump(out, symbols);
}
