#pragma once

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include "id.h"
#include "symbol_table.h"

class Relation {
private:
    std::vector<id_t> data;
    int arity;

public:
    Relation(int arity) : arity(arity) {}

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

    const std::vector<id_t>& get_data() const {
        return data;
    }
};

class Database {
private:
    std::unordered_map<symbol_t, Relation> relations;

public:
    void add_relation(symbol_t name, int arity) {
        relations.emplace(name, Relation(arity));
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
};
