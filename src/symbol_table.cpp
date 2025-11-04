#include <cassert>

#include "symbol_table.h"

static const std::string opaque = "<opaque>";

Symbol SymbolTable::intern(const std::string& str)
{
    auto it = map.find(str);
    if (it != map.end())
    {
        return it->second;
    }

    Symbol symbol = next_id++;
    map[str] = symbol;
    return symbol;
}

Symbol SymbolTable::create_opaque()
{
    return next_id++;
}

const std::string& SymbolTable::get_string(Symbol symbol) const
{
    for (auto const& [key, val] : map)
        if (val == symbol) return key;

    return opaque;
}

bool SymbolTable::has_symbol(Symbol symbol) const
{
    return symbol < map.size();
}

size_t SymbolTable::size() const
{
    return next_id;
}
