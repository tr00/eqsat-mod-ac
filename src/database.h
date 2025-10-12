#pragma once

#include <cassert>
#include <memory>
#include <stdexcept>

#include "id.h"
#include "indices/abstract_index.h"
#include "relations/abstract_relation.h"
#include "relations/relation_ac.h"
#include "relations/row_store.h"
#include "symbol_table.h"

/**
 * @brief Key for indexing relations by operator symbol and permutation
 *
 * Used as a composite key to uniquely identify trie indices in the database.
 * Each index is specific to both an operator symbol and a particular permutation
 * of the tuple fields, enabling efficient query processing with different
 * field orderings.
 */
using IndexKey = std::pair<Symbol, uint32_t>;

/**
 * @brief A database containing relations and their associated indices
 *
 * The Database class manages a collection of named relations and their trie-based
 * indices for efficient query processing. Each relation stores tuples for a specific
 * operator symbol, and indices provide fast lookup capabilities with different
 * tuple field orderings via permutations.
 *
 * The database supports:
 * - Creating and managing relations with different arities
 * - Adding tuples to relations
 * - Creating trie indices for efficient querying
 * - Multiple indices per relation with different field permutations
 *
 * Example usage:
 * ```cpp
 * Database db;
 * db.add_relation(add_symbol, 2);           // Create binary relation for addition
 * db.add_tuple(add_symbol, {1, 2});         // Add tuple (1, 2)
 * db.add_index(add_symbol, 0);              // Create identity permutation index
 * db.build_indices();                       // Populate indices with data
 * ```
 */
class Database
{
  private:
    HashMap<Symbol, AbstractRelation> relations;
    HashMap<IndexKey, AbstractIndex> indices;

    AbstractRelation *get_relation(Symbol rel_name)
    {
        auto it = relations.find(rel_name);
        if (it == relations.end())
            return nullptr;

        return &it->second;
    }

    const AbstractRelation *get_relation(Symbol rel_name) const
    {
        auto it = relations.find(rel_name);
        if (it == relations.end())
            return nullptr;

        return &it->second;
    }

  public:
    /**
     * @brief Create a new relation in the database
     *
     * @param name The operator symbol for the relation
     * @param arity The number of fields in each tuple (must be > 0)
     *
     * @note If a relation with the same name already exists, this is a no-op
     */
    void create_relation(Symbol name, int arity)
    {
        relations.emplace(name, AbstractRelation(RowStore(name, arity)));
    }

    void create_relation_ac(Symbol name)
    {
        relations.emplace(name, AbstractRelation(RelationAC(name)));
    }

    /**
     * @brief Add a tuple to an existing relation
     *
     * @param relation_name The operator symbol of the target relation
     * @param tuple The tuple to add, must match relation's arity
     * @throws std::runtime_error if relation doesn't exist
     * @throws std::invalid_argument if tuple size doesn't match relation arity
     */
    void add_tuple(Symbol relation_name, const Vec<id_t>& tuple)
    {
        auto it = relations.find(relation_name);
        if (it == relations.end())
        {
            throw std::runtime_error("Relation not found");
        }
        it->second.add_tuple(tuple);
    }

    /**
     * @brief Check if a relation exists in the database
     *
     * @param name The operator symbol to check for
     * @return true if relation exists, false otherwise
     */
    bool has_relation(Symbol name) const
    {
        return relations.find(name) != relations.end();
    }

    /**
     * @brief Create an empty trie index for a relation with specific permutation
     *
     * Creates a new trie node that will be populated when build_indices() is called.
     * The permutation_id determines how tuple fields are ordered in the index.
     *
     * @param name The operator symbol for the relation to index
     * @param perm The lexicographic permutation index for field ordering
     *
     * @note If an index with the same key already exists, it will be replaced
     */
    void create_index(Symbol name, uint32_t perm)
    {
        if (get_relation(name)->get_kind() == RELATION_AC)
            perm = 0;

        IndexKey key{name, perm};

        auto trie_node = std::make_shared<TrieNode>();
        TrieIndex trie_index(trie_node);

        indices[key] = AbstractIndex(trie_index);
    }

    /**
     * @brief Retrieve a copy of a trie index for querying
     *
     * @param operator_symbol The operator symbol for the relation
     * @param permutation_id The permutation index for the desired field ordering
     * @return A COPY of the AbstractIndex
     *
     * @note Returns a copy of the index to allow independent simultaneous traversals.
     *       The underlying TrieNode data is shared via shared_ptr, so only the traversal
     *       state is duplicated, not the actual trie data.
     *       Asserts if the index doesn't exist.
     */
    AbstractIndex get_index(Symbol name, uint32_t perm) const
    {
        if (get_relation(name)->get_kind() == RELATION_AC)
            perm = 0;

        IndexKey key(name, perm);
        auto it = indices.find(key);
        assert(it != indices.end() && "Index not found");
        return it->second; // Returns a copy
    }

    /**
     * @brief Check if an index exists for the given operator and permutation
     *
     * @param operator_symbol The operator symbol to check
     * @param permutation_id The permutation index to check
     * @return true if index exists, false otherwise
     */
    bool has_index(Symbol name, uint32_t perm) const
    {
        if (get_relation(name)->get_kind() == RELATION_AC)
            perm = 0;

        IndexKey key(name, perm);
        return indices.find(key) != indices.end();
    }

    /**
     * @brief Clear all trie indices from the database
     *
     * Removes all indices while keeping the relations intact.
     * Useful for rebuilding indices or freeing memory.
     */
    void clear_indices();

    /**
     * @brief Populate all indices with data from their corresponding relations
     *
     * Iterates through all created indices and populates them with tuples
     * from their associated relations, applying the appropriate field
     * permutations as specified by the permutation_id.
     *
     * @note Must be called after creating indices and adding tuples to relations
     */
    void populate_indices();
};
