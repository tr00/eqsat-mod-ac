#include "database.h"

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
