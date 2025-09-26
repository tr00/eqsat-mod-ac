#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

using symbol_t = uint32_t;

class SymbolTable
{
  private:
    std::unordered_map<std::string, symbol_t> string_to_symbol;
    std::vector<std::string> symbol_to_string;
    symbol_t next_id;

  public:
    SymbolTable();

    symbol_t intern(const std::string &str);
    const std::string &get_string(symbol_t symbol) const;
    bool has_symbol(symbol_t symbol) const;
    size_t size() const;
};