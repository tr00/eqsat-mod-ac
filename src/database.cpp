#include "database.h"
#include "permutation.h"
#include <cassert>
#include <vector>
#include <algorithm>

void Relation::dump(std::ostream& os, const SymbolTable& symbol_table) const {
    os << "relation " << symbol_table.get_string(operator_symbol)
       << " with arity " << arity << std::endl;

    for (size_t i = 0; i < size(); ++i) {
        for (int j = 0; j < arity; ++j) {
            if (j > 0) os << " ";
            os << get(i, j);
        }
        os << std::endl;
    }
}

void Database::build_indices() {
    for (auto& [index_key, index] : indices) {
        auto [rel_name, perm] = index_key;
        auto index_ref = index;

        auto relation = get_relation(rel_name);
        assert(relation);
        size_t arity = relation->get_arity();

        // precompute the permutation indices once
        std::vector<uint32_t> indices_range(arity);
        for (size_t i = 0; i < arity; ++i) {
            indices_range[i] = static_cast<uint32_t>(i);
        }
        std::vector<uint32_t> perm_indices = index_to_permutation(perm, indices_range);

        std::vector<id_t> buffer(arity);
        relation->for_each_tuple([&](const id_t* tuple, size_t i) {
            std::copy(tuple, tuple + arity, buffer.begin());

            apply_permutation(perm_indices, buffer);

            index_ref->insert_path(buffer);
        });
    }
}

void Database::clear_indices() {
    indices.clear();
}
