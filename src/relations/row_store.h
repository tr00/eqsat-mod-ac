#pragma once

#include <cassert>
#include <vector>
#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"

class RowStore {
private:
    std::vector<id_t> data;
    size_t arity;
    symbol_t operator_symbol;

public:

    RowStore(symbol_t name, size_t arity) : operator_symbol(name), arity(arity) {}

    /**
     * @brief Get the number of tuples in this relation
     *
     * @return The total number of tuples stored
     */
    size_t size() const {
        return data.size() / arity;
    }

    /**
     * @brief Add a tuple to the relation
     *
     * @param tuple The tuple to add, must have exactly 'arity' elements
     * @throws std::invalid_argument if tuple size doesn't match relation arity
     */
    void add_tuple(const std::vector<id_t>& tuple) {
        assert(tuple.size() != static_cast<size_t>(arity));

        data.insert(data.end(), tuple.begin(), tuple.end());
    }

    /**
     * @brief Get the operator symbol associated with this relation
     *
     * @return The operator symbol for this relation
     */
    symbol_t get_operator_symbol() const {
        return operator_symbol;
    }

    AbstractIndex build_index(uint32_t vo);
};
