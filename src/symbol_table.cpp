#include "symbol_table.h"

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

const std::string& SymbolTable::get_string(Symbol symbol) const
{
    for (auto const& [key, val] : map)
        if (val == symbol)
            return key;

    assert(0);
}

bool SymbolTable::has_symbol(Symbol symbol) const
{
    return symbol < map.size();
}

size_t SymbolTable::size() const
{
    return map.size();
}
