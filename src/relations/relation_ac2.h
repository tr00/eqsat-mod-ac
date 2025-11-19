#pragma once

#include <fstream>
#include <utility>

#include "handle.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"
#include "utils/multiset.h"
#include "utils/vec.h"

class RelationAC2
{
  private:
    Vec<std::pair<id_t, Multiset>> data;
    Symbol symbol;

    bool insert(std::pair<id_t, Multiset> tuple);
    void sort();
    bool canonicalize(const Handle egraph);
    bool congruence(Handle egraph);
    bool flatten();
    bool unflatten();

  public:
    RelationAC2(Symbol symbol)
        : data()
        , symbol(symbol)
    {
    }

    Symbol get_symbol() const
    {
        return symbol;
    }

    size_t size() const
    {
        return data.size();
    }

    void add_tuple(const Vec<id_t>& tuple);
    void add_tuple(id_t id, Multiset mset);

    AbstractIndex populate_index();

    bool rebuild(Handle egraph);

    void dump(std::ofstream& out, const SymbolTable& symbols) const;
};
