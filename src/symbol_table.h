#pragma once

#include <cstdint>
#include <string>

#include "utils/hashmap.h"

using Symbol = uint32_t;

class SymbolTable
{
  private:
    HashMap<std::string, Symbol> map;
    Symbol next_id;

  public:
    SymbolTable()
        : next_id(0)
    {
    }

    Symbol intern(const std::string& str);

    const std::string& get_string(Symbol symbol) const;
    bool has_symbol(Symbol symbol) const;
    size_t size() const;
};
