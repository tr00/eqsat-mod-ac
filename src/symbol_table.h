#pragma once

#include <string>

#include "types.h"

namespace eqsat
{

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

    /**
     * @brief Create an opaque symbol with a unique ID but no associated string.
     * @return Symbol identifier for the opaque symbol
     *
     * Opaque symbols have unique IDs but return "<opaque>" when get_string() is called.
     * They are useful for representing generated or anonymous symbols without string names.
     */
    Symbol create_opaque();

    const std::string& get_string(Symbol symbol) const;
    bool has_symbol(Symbol symbol) const;
    size_t size() const;
};

} // namespace eqsat
