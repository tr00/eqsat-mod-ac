#pragma once

#include "handle.h"
#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"
#include "utils/multiset.h"
#include <fstream>
#include <functional>
#include <memory>

class RelationAC
{
  private:
    // eclass-id < term-id < mset
    std::shared_ptr<HashMap<id_t, HashMap<id_t, Multiset>>> data;
    size_t nterms;
    Symbol symbol;

  public:
    RelationAC(Symbol symbol)
        : data(std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>())
        , nterms(0)
        , symbol(symbol)
    {
    }

    Symbol get_symbol() const
    {
        return symbol;
    }

    size_t size() const
    {
        return nterms;
    }

    void add_tuple(const Vec<id_t>& tuple);
    void add_tuple(id_t id, Multiset mset);

    /**
     * @brief Create an empty multiset index for this AC relation
     *
     * @return An empty AbstractIndex containing a MultisetIndex
     * @note Permutation parameter is ignored for AC relations
     */
    AbstractIndex create_index()
    {
        return AbstractIndex(MultisetIndex(data));
    }

    AbstractIndex populate_index()
    {
        return AbstractIndex(MultisetIndex(data));
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
