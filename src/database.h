#pragma once

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "id.h"
#include "symbol_table.h"
#include "trie_index.h"

/**
 * @brief A relation stores tuples of fixed arity for a given operator symbol
 * 
 * Relations are the fundamental data structure for storing facts in the database.
 * Each relation is associated with an operator symbol and has a fixed arity.
 * Tuples are stored consecutively in a flat vector for efficient memory usage
 * and cache performance.
 * 
 * Example usage:
 * ```cpp
 * Relation rel(add_symbol, 2);  // Binary relation for addition
 * rel.add_tuple({1, 2});        // Add tuple (1, 2)
 * rel.add_tuple({3, 4});        // Add tuple (3, 4)
 * id_t first = rel.get(0, 0);   // Get first element of first tuple: 1
 * ```
 */
class Relation {
private:
    std::vector<id_t> data;
    int arity;
    symbol_t operator_symbol;

public:
    /**
     * @brief Construct a new Relation with given operator symbol and arity
     * 
     * @param op_symbol The operator symbol this relation represents
     * @param arity The number of fields in each tuple (must be > 0)
     */
    Relation(symbol_t op_symbol, int arity) : arity(arity), operator_symbol(op_symbol) {}

    /**
     * @brief Add a tuple to the relation
     * 
     * @param tuple The tuple to add, must have exactly 'arity' elements
     * @throws std::invalid_argument if tuple size doesn't match relation arity
     */
    void add_tuple(const std::vector<id_t>& tuple) {
        if (tuple.size() != static_cast<size_t>(arity)) {
            throw std::invalid_argument("Tuple size must match relation arity");
        }
        data.insert(data.end(), tuple.begin(), tuple.end());
    }

    /**
     * @brief Get a specific field from a specific tuple
     * 
     * @param tuple_index The index of the tuple (0-based)
     * @param field_index The index of the field within the tuple (0-based)
     * @return The id_t value at the specified position
     * @throws std::out_of_range if either index is out of bounds
     */
    id_t get(size_t tuple_index, int field_index) const {
        if (tuple_index >= size() || field_index >= arity) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[tuple_index * arity + field_index];
    }

    /**
     * @brief Get an entire tuple as a vector
     * 
     * @param tuple_index The index of the tuple to retrieve (0-based)
     * @return A vector containing all fields of the specified tuple
     * @throws std::out_of_range if tuple_index is out of bounds
     */
    std::vector<id_t> get_tuple(size_t tuple_index) const {
        if (tuple_index >= size()) {
            throw std::out_of_range("Tuple index out of bounds");
        }
        auto start = data.begin() + tuple_index * arity;
        return std::vector<id_t>(start, start + arity);
    }

    /**
     * @brief Get the number of tuples in this relation
     * 
     * @return The total number of tuples stored
     */
    size_t size() const {
        return data.size() / arity;
    }

    /**
     * @brief Get the arity (number of fields per tuple) of this relation
     * 
     * @return The arity of this relation
     */
    int get_arity() const {
        return arity;
    }

    /**
     * @brief Get the operator symbol associated with this relation
     * 
     * @return The operator symbol for this relation
     */
    symbol_t get_operator_symbol() const {
        return operator_symbol;
    }

    /**
     * @brief Get direct access to the underlying data vector
     * 
     * @return Const reference to the flat data vector containing all tuples
     * @warning Use with caution - data is stored as consecutive tuples
     */
    const std::vector<id_t>& get_data() const {
        return data;
    }

    /**
     * @brief Print the relation contents to an output stream
     * 
     * @param os The output stream to write to
     * @param symbol_table Symbol table for converting symbols to strings
     */
    void dump(std::ostream& os, const SymbolTable& symbol_table) const;

    /**
     * @brief Iterate over all tuples with a callback function
     * 
     * Provides efficient iteration over tuples without creating temporary vectors.
     * The callback receives a pointer to the first element of each tuple and the tuple index.
     * 
     * @tparam Func Function or lambda type taking (const id_t*, size_t)
     * @param func Callback function called for each tuple
     * 
     * Example usage:
     * ```cpp
     * relation.for_each_tuple([](const id_t* tuple, size_t index) {
     *     std::cout << "Tuple " << index << ": " << tuple[0] << ", " << tuple[1] << std::endl;
     * });
     * ```
     */
    template<typename Func>
    void for_each_tuple(Func func) const {
        const id_t* ptr = data.data();
        for (size_t i = 0; i < size(); ++i) {
            func(ptr + i * arity, i);
        }
    }
};

/**
 * @brief Key for indexing relations by operator symbol and permutation
 * 
 * Used as a composite key to uniquely identify trie indices in the database.
 * Each index is specific to both an operator symbol and a particular permutation
 * of the tuple fields, enabling efficient query processing with different
 * field orderings.
 */
struct IndexKey {
    symbol_t operator_symbol;
    uint32_t permutation_id;

    bool operator==(const IndexKey& other) const {
        return operator_symbol == other.operator_symbol && permutation_id == other.permutation_id;
    }
};

// Hash function for IndexKey
struct IndexKeyHash {
    std::size_t operator()(const IndexKey& key) const {
        return std::hash<uint64_t>{}((static_cast<uint64_t>(key.operator_symbol) << 32) | key.permutation_id);
    }
};

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
class Database {
private:
    std::unordered_map<symbol_t, Relation> relations;
    std::unordered_map<IndexKey, std::shared_ptr<TrieNode>, IndexKeyHash> indices;

public:
    /**
     * @brief Create a new relation in the database
     * 
     * @param name The operator symbol for the relation
     * @param arity The number of fields in each tuple (must be > 0)
     * 
     * @note If a relation with the same name already exists, this is a no-op
     */
    void add_relation(symbol_t name, int arity) {
        relations.emplace(name, Relation(name, arity));
    }

    /**
     * @brief Add a tuple to an existing relation
     * 
     * @param relation_name The operator symbol of the target relation
     * @param tuple The tuple to add, must match relation's arity
     * @throws std::runtime_error if relation doesn't exist
     * @throws std::invalid_argument if tuple size doesn't match relation arity
     */
    void add_tuple(symbol_t relation_name, const std::vector<id_t>& tuple) {
        auto it = relations.find(relation_name);
        if (it == relations.end()) {
            throw std::runtime_error("Relation not found");
        }
        it->second.add_tuple(tuple);
    }

    /**
     * @brief Get a mutable pointer to a relation
     * 
     * @param name The operator symbol of the relation
     * @return Pointer to the relation, or nullptr if not found
     */
    Relation* get_relation(symbol_t name) {
        auto it = relations.find(name);
        return (it != relations.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Get a const pointer to a relation
     * 
     * @param name The operator symbol of the relation
     * @return Const pointer to the relation, or nullptr if not found
     */
    const Relation* get_relation(symbol_t name) const {
        auto it = relations.find(name);
        return (it != relations.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Check if a relation exists in the database
     * 
     * @param name The operator symbol to check for
     * @return true if relation exists, false otherwise
     */
    bool has_relation(symbol_t name) const {
        return relations.find(name) != relations.end();
    }

    /**
     * @brief Create an empty trie index for a relation with specific permutation
     * 
     * Creates a new trie node that will be populated when build_indices() is called.
     * The permutation_id determines how tuple fields are ordered in the index.
     * 
     * @param operator_symbol The operator symbol for the relation to index
     * @param permutation_id The lexicographic permutation index for field ordering
     * 
     * @note If an index with the same key already exists, it will be replaced
     */
    void add_index(symbol_t operator_symbol, uint32_t permutation_id) {
        IndexKey key{operator_symbol, permutation_id};

        auto trie_node = std::make_shared<TrieNode>();

        indices[key] = trie_node;
    }

    /**
     * @brief Retrieve a trie index for querying
     * 
     * @param operator_symbol The operator symbol for the relation
     * @param permutation_id The permutation index for the desired field ordering
     * @return Shared pointer to TrieIndex wrapper, or nullptr if index doesn't exist
     * 
     * @note Creates a new TrieIndex wrapper around the stored TrieNode
     */
    std::shared_ptr<TrieIndex> get_index(symbol_t operator_symbol, uint32_t permutation_id) const {
        IndexKey key{operator_symbol, permutation_id};
        auto it = indices.find(key);
        if (it != indices.end()) {
            return std::make_shared<TrieIndex>(*it->second);
        } else {
            return nullptr;
        }
    }

    /**
     * @brief Check if an index exists for the given operator and permutation
     * 
     * @param operator_symbol The operator symbol to check
     * @param permutation_id The permutation index to check
     * @return true if index exists, false otherwise
     */
    bool has_index(symbol_t operator_symbol, uint32_t permutation_id) const {
        IndexKey key{operator_symbol, permutation_id};
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
    void build_indices();
};
