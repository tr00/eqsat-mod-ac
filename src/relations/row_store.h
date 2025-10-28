#pragma once

#include <cassert>
#include <fstream>

#include "handle.h"
#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"

class RowStore
{
  private:
    Vec<id_t> data;
    size_t arity;
    Symbol symbol;

  public:
    RowStore(Symbol symbol, size_t arity)
        : arity(arity)
        , symbol(symbol)
    {
    }

    /**
     * @brief Get the number of tuples in this relation
     *
     * @return The total number of tuples stored
     */
    size_t size() const
    {
        return data.size() / arity;
    }

    /**
     * @brief Add a tuple to the relation
     *
     * @param tuple The tuple to add, must have exactly 'arity' elements
     * @throws std::invalid_argument if tuple size doesn't match relation arity
     */
    void add_tuple(const Vec<id_t>& tuple)
    {
        assert(tuple.size() == static_cast<size_t>(arity));

        data.insert(data.end(), tuple.begin(), tuple.end());
    }

    /**
     * @brief Get the operator symbol associated with this relation
     *
     * @return The operator symbol for this relation
     */
    Symbol get_symbol() const
    {
        return symbol;
    }

    AbstractIndex populate_index(uint32_t vo);

    /**
     * @brief Rebuild the relation by detecting and unifying duplicate entries
     *
     * Sorts all tuples by their first (arity-1) attributes and detects neighboring
     * tuples with identical arguments but different e-class IDs. When found, calls
     * the unify_callback to merge the e-classes.
     *
     * @param unify_callback Function to call when two IDs need to be unified
     * @return true if any unifications were performed, false otherwise
     */
    bool rebuild(Handle handle);

    /**
     * @brief Dump the relation contents to a file
     *
     * @param out Output file stream
     * @param symbols Symbol table for resolving operator names
     */
    void dump(std::ofstream& out, const SymbolTable& symbols) const;
};
