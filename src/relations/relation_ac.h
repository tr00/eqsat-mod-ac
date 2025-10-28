#pragma once

#include <fstream>
#include <memory>

#include "handle.h"
#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"
#include "utils/multiset.h"

class RelationAC
{
  private:
    // eclass-id < term-id < mset
    std::shared_ptr<HashMap<id_t, Multiset>> data;
    Vec<id_t> ids;
    Symbol symbol;

  public:
    RelationAC(Symbol symbol)
        : data(std::make_shared<HashMap<id_t, Multiset>>())
        , ids()
        , symbol(symbol)
    {
    }

    Symbol get_symbol() const
    {
        return symbol;
    }

    size_t size() const
    {
        return ids.size();
    }

    void add_tuple(const Vec<id_t>& tuple);
    void add_tuple(id_t id, Multiset mset);

    AbstractIndex populate_index()
    {
        return AbstractIndex(MultisetIndex(symbol, data));
    }

    bool rebuild(Handle handle);

    /**
     * @brief Dump the relation contents to a file
     *
     * @param out Output file stream
     * @param symbols Symbol table for resolving operator names
     */
    void dump(std::ofstream& out, const SymbolTable& symbols) const;
};
