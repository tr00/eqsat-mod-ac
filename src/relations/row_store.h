#pragma once

#include <cassert>
#include <functional>

#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"

class RowStore
{
  private:
    Vec<id_t> data;
    size_t arity;
    Symbol operator_symbol;

  public:
    RowStore(Symbol name, size_t arity) : operator_symbol(name), arity(arity)
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
    Symbol get_operator_symbol() const
    {
        return operator_symbol;
    }

    AbstractIndex populate_index(uint32_t vo);

    /**
     * @brief Create an empty trie index for this relation
     *
     * @param perm The permutation index for field ordering
     * @return An empty AbstractIndex containing a TrieIndex
     */
    AbstractIndex create_index(uint32_t perm);

    using canon_t = std::function<id_t(id_t)>;
    using unify_t = std::function<id_t(id_t, id_t)>;

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
    bool rebuild(canon_t canonicalize, unify_t unify);
};
