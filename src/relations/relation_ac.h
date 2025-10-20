#pragma once

#include "id.h"
#include "indices/abstract_index.h"
#include "symbol_table.h"
#include "utils/multiset.h"
#include <memory>

class RelationAC
{
  private:
    // eclass-id < term-id < mset
    std::shared_ptr<HashMap<id_t, HashMap<id_t, Multiset>>> data;
    Symbol symbol;

  public:
    RelationAC(Symbol symbol)
        : data(std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>())
        , symbol(symbol)
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
        id_t eclass = tuple.back();
        id_t term = static_cast<id_t>(size());

        Vec<id_t> copy = tuple;
        copy.pop_back();

        if (!data->contains(eclass)) data->emplace(eclass, HashMap<id_t, Multiset>());

        (*data)[eclass].emplace(term, std::move(copy));
    }

    /**
     * @brief Create an empty multiset index for this AC relation
     *
     * @return An empty AbstractIndex containing a MultisetIndex
     * @note Permutation parameter is ignored for AC relations
     */
    AbstractIndex create_index()
    {
        return AbstractIndex(MultisetIndex(data));
    }

    AbstractIndex populate_index()
    {
        return AbstractIndex(MultisetIndex(data));
    }
};
