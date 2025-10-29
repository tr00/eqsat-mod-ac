#pragma once

#include <cassert>
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
 * Used as a composite key to uniquely identify indices in the database.
 * Each index is specific to both an operator symbol and a particular permutation
 * of the tuple fields, enabling efficient query processing with different
 * field orderings.
 */
using IndexKey = std::pair<Symbol, uint32_t>;

/**
 * @brief Database for equality saturation with support for both standard and AC operators
 *
 * # Overview
 *
 * The Database manages relations and indices for efficient conjunctive query execution
 * in an equality saturation framework. It supports two types of operators:
 * - **Standard operators**: Use ordered tuple storage (RowStore) and trie indices
 * - **AC operators**: Use multiset-based storage (RelationAC) and multiset indices
 *
 * # Architecture
 *
 * The database consists of three main components:
 *
 * 1. **Relations**: Store tuples representing terms in the e-graph
 *    - Each relation is identified by an operator symbol
 *    - Relations can be standard (RowStore) or AC (RelationAC)
 *    - Type-erased via AbstractRelation for uniform interface
 *
 * 2. **Indices**: Enable efficient query evaluation via select/project operations
 *    - Standard operators: TrieIndex with support for multiple permutations
 *    - AC operators: MultisetIndex (permutation-invariant, always normalized to 0)
 *    - Type-erased via AbstractIndex for uniform traversal interface
 *
 * 3. **Index Keys**: (Symbol, permutation) pairs for index lookup
 *    - Permutation encoded as lexicographic index using factorial number system
 *    - For AC operators, permutation is always 0 (commutative matching)
 *
 * # Data Model
 *
 * **Tuple Format**: Relations store tuples as `op(arg1, arg2, ..., argN; eclass_id)`
 * where the **last column is always the e-class identifier**. This is a critical invariant.
 *
 * Examples:
 * - `add(1, 2; 10)` means `add(1, 2)` belongs to e-class 10
 * - `mul(5, 3; 20)` means `mul(5, 3)` belongs to e-class 20
 * - For AC operators: `mul_ac({1, 2}; 30)` where {1, 2} is a multiset
 *
 * # Permutations
 *
 * Standard relations support multiple indices with different attribute orderings (permutations).
 * Permutations are encoded as integers using the factorial number system:
 *
 * For arity 3: [0,1,2]=0, [0,2,1]=1, [1,0,2]=2, [1,2,0]=3, [2,0,1]=4, [2,1,0]=5
 *
 * The e-class ID column participates in permutation for efficient query planning.
 *
 * # AC Operator Handling
 *
 * AC (Associative-Commutative) operators require special treatment:
 * - Arguments stored as **multisets** rather than ordered tuples
 * - Only one index per AC relation (permutation always normalized to 0)
 * - Pattern matching is order-independent: `mul(x, 2)` matches both `mul(2, x)` and `mul(x, 2)`
 * - `has_index()` and `get_index()` normalize any permutation to 0 for AC relations
 *
 * # Interface
 *
 * ## Relation Management
 * - `create_relation(symbol, arity)`: Create standard relation
 * - `create_relation_ac(symbol)`: Create AC relation
 * - `add_tuple(symbol, tuple)`: Insert tuple into relation
 * - `has_relation(symbol)`: Check relation existence
 *
 * ## Index Management
 * - `populate_index(symbol, perm)`: Create and populate index (atomic operation)
 * - `has_index(symbol, perm)`: Check if index exists
 * - `get_index(symbol, perm)`: Retrieve index copy for traversal
 * - `clear_indices()`: Remove all indices (relations preserved)
 *
 * ## Rebuild
 * - `rebuild(handle)`: Detect and unify equivalent terms across all relations
 *   - Scans for tuples with same arguments but different e-class IDs
 *   - Calls handle's unify to merge e-classes
 *   - Returns true if any unifications occurred
 *
 * # Usage Example
 *
 * ```cpp
 * Theory theory;
 * Symbol add = theory.add_operator("add", 2);
 * Symbol mul = theory.add_operator("mul", AC); // AC operator
 *
 * Database db;
 * db.create_relation(add, 3);      // Binary op: add(arg1, arg2; eclass_id)
 * db.create_relation_ac(mul);      // AC op: mul({args...}; eclass_id)
 *
 * // Add terms
 * db.add_tuple(add, {1, 2, 10});   // add(1,2) in e-class 10
 * db.add_tuple(mul, {1, 2, 20});   // mul(1,2) in e-class 20
 *
 * // Create indices for query execution
 * db.populate_index(add, 0);       // Identity permutation [0,1,2]
 * db.populate_index(add, 2);       // Swapped permutation [1,0,2]
 * db.populate_index(mul, 0);       // AC: any permutation → 0
 *
 * // Check indices
 * assert(db.has_index(add, 0));
 * assert(db.has_index(add, 2));
 * assert(db.has_index(mul, 999));  // Any permutation works for AC
 *
 * // Get index for traversal
 * AbstractIndex idx = db.get_index(add, 0);
 * ```
 *
 * # Implementation Notes
 *
 * - Relations are stored in a HashMap keyed by Symbol
 * - Indices are stored in a HashMap keyed by IndexKey (Symbol, permutation)
 * - `get_index()` returns a **copy** to allow independent simultaneous traversals
 *   - Underlying data is shared via shared_ptr (cheap copy)
 *   - Only traversal state (history stack) is duplicated
 * - `populate_index()` is atomic: creates and populates in one call
 * - AC relations always normalize permutation to 0 in all operations
 */
class Database
{
  private:
    HashMap<Symbol, AbstractRelation> relations;
    HashMap<IndexKey, AbstractIndex> indices;

    AbstractRelation *get_relation(Symbol rel_name)
    {
        auto it = relations.find(rel_name);
        if (it == relations.end()) return nullptr;

        return &it->second;
    }

    const AbstractRelation *get_relation(Symbol rel_name) const
    {
        auto it = relations.find(rel_name);
        if (it == relations.end()) return nullptr;

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
        if (get_relation(name)->get_kind() == RELATION_AC) perm = static_cast<uint32_t>(-1);

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
        if (get_relation(name)->get_kind() == RELATION_AC) perm = static_cast<uint32_t>(-1);

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
     * @brief Create and populate an index for a relation with specific permutation
     *
     * Delegates to the relation to create and populate the appropriate index type:
     * - RowStore creates a TrieIndex with permuted data
     * - RelationAC creates a MultisetIndex (ignores permutation)
     *
     * @param name The operator symbol for the relation to index
     * @param perm The lexicographic permutation index for field ordering
     *
     * @note If an index with the same key already exists, it will be replaced
     * @note For AC relations, permutation is always normalized to 0
     */
    void populate_index(Symbol name, uint32_t perm)
    {
        if (get_relation(name)->get_kind() == RELATION_AC) perm = static_cast<uint32_t>(-1);

        IndexKey key{name, perm};

        auto *relation = get_relation(name);
        assert(relation != nullptr && "Relation not found");

        indices[key] = relation->populate_index(perm);
    }

    /**
     * @brief Rebuild all relations by detecting and unifying duplicate entries
     *
     * Iterates through all relations in the database. For each relation, sorts all
     * tuples by their first (arity-1) attributes and detects neighboring tuples with
     * identical arguments but different e-class IDs. When found, calls the unify
     * callback to merge the e-classes.
     *
     * @param canonicalize Function to canonicalize an ID
     * @param unify Function to call when two IDs need to be unified
     * @return true if any unifications were performed in any relation, false otherwise
     *
     * @note For AC relations, rebuild is a no-op
     */
    bool rebuild(Handle handle);

    /**
     * @brief Dump all relations to a file
     *
     * @param filename The path to the output file
     * @param symbols Symbol table for resolving operator names
     */
    void dump_to_file(const std::string& filename, const SymbolTable& symbols) const;
};
