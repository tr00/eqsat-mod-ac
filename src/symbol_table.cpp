#include "symbol_table.h"

SymbolTable::SymbolTable() : next_id(0)
{
}

symbol_t SymbolTable::intern(const std::string& str)
{
    auto it = string_to_symbol.find(str);
    if (it != string_to_symbol.end())
    {
        return it->second;
    }

    symbol_t new_symbol = next_id++;
    string_to_symbol[str] = new_symbol;
    symbol_to_string.push_back(str);
    return new_symbol;
}

const std::string& SymbolTable::get_string(symbol_t symbol) const
{
    return symbol_to_string[symbol];
}

bool SymbolTable::has_symbol(symbol_t symbol) const
{
    return symbol < symbol_to_string.size();
}

size_t SymbolTable::size() const
{
    return symbol_to_string.size();
}