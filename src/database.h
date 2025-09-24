#pragma once

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "id.h"
#include "symbol_table.h"
#include "trie_index.h"

class Relation {
private:
    std::vector<id_t> data;
    int arity;
    symbol_t operator_symbol;

public:
    Relation(symbol_t op_symbol, int arity) : arity(arity), operator_symbol(op_symbol) {}

    void add_tuple(const std::vector<id_t>& tuple) {
        if (tuple.size() != static_cast<size_t>(arity)) {
            throw std::invalid_argument("Tuple size must match relation arity");
        }
        data.insert(data.end(), tuple.begin(), tuple.end());
    }

    id_t get(size_t tuple_index, int field_index) const {
        if (tuple_index >= size() || field_index >= arity) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[tuple_index * arity + field_index];
    }

    std::vector<id_t> get_tuple(size_t tuple_index) const {
        if (tuple_index >= size()) {
            throw std::out_of_range("Tuple index out of bounds");
        }
        auto start = data.begin() + tuple_index * arity;
        return std::vector<id_t>(start, start + arity);
    }

    size_t size() const {
        return data.size() / arity;
    }

    int get_arity() const {
        return arity;
    }

    symbol_t get_operator_symbol() const {
        return operator_symbol;
    }

    const std::vector<id_t>& get_data() const {
        return data;
    }

    void dump(std::ostream& os, const SymbolTable& symbol_table) const;
};

// Key for indexing: combines operator symbol and permutation id
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

class Database {
private:
    std::unordered_map<symbol_t, Relation> relations;
    std::unordered_map<IndexKey, std::shared_ptr<TrieIndex>, IndexKeyHash> indices;

public:
    void add_relation(symbol_t name, int arity) {
        relations.emplace(name, Relation(name, arity));
    }

    void add_tuple(symbol_t relation_name, const std::vector<id_t>& tuple) {
        auto it = relations.find(relation_name);
        if (it == relations.end()) {
            throw std::runtime_error("Relation not found");
        }
        it->second.add_tuple(tuple);
    }

    Relation* get_relation(symbol_t name) {
        auto it = relations.find(name);
        return (it != relations.end()) ? &it->second : nullptr;
    }

    const Relation* get_relation(symbol_t name) const {
        auto it = relations.find(name);
        return (it != relations.end()) ? &it->second : nullptr;
    }

    bool has_relation(symbol_t name) const {
        return relations.find(name) != relations.end();
    }

    // Create an empty index for a given operator symbol and permutation
    void add_index(symbol_t operator_symbol, uint32_t permutation_id) {
        IndexKey key{operator_symbol, permutation_id};

        auto trie_node = std::make_shared<TrieNode>();
        auto trie_index = std::make_shared<TrieIndex>(*trie_node);

        indices[key] = trie_index;
    }

    // Retrieve an index by operator symbol and permutation id
    std::shared_ptr<TrieIndex> get_index(symbol_t operator_symbol, uint32_t permutation_id) const {
        IndexKey key{operator_symbol, permutation_id};
        auto it = indices.find(key);
        return (it != indices.end()) ? it->second : nullptr;
    }

    // Check if an index exists for the given operator symbol and permutation
    bool has_index(symbol_t operator_symbol, uint32_t permutation_id) const {
        IndexKey key{operator_symbol, permutation_id};
        return indices.find(key) != indices.end();
    }
};
