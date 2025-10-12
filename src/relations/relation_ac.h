#pragma once

#include "id.h"
#include "indices/abstract_index.h"
#include "indices/multiset_index.h"
#include "symbol_table.h"
#include "utils/multiset.h"
#include <memory>

// what about ids?

class RelationAC
{
  private:
    // term --> mset
    std::shared_ptr<HashMap<id_t, Multiset>> data;
    Symbol symbol;

  public:
    RelationAC(Symbol symbol) : data(std::make_shared<HashMap<id_t, Multiset>>()), symbol(symbol)
    {
    }

    Symbol get_operator_symbol() const
    {
        return symbol;
    }

    size_t size() const
    {
        return data->size();
    }

    void add_tuple(const Vec<id_t>& tuple)
    {
        id_t new_term_id = static_cast<id_t>(size());
        data->emplace(new_term_id, tuple);
    }

    AbstractIndex populate_index()
    {
        return AbstractIndex(MultisetIndex(data));
    }
};
