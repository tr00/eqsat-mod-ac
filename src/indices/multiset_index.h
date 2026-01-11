#pragma once

#include <optional>

#include "../sets/abstract_set.h"
#include "../utils/multiset.h"
#include "enode.h"
#include "id.h"
#include "symbol_table.h"

namespace eqsat
{

class MultisetIndex
{
  private:
    // term-id < children... [ < eclass-id ]
    Vec<id_t> history;
    HashMap<id_t, Multiset> data;
    std::optional<Multiset *> mset;
    Symbol symbol;

  public:
    MultisetIndex(Symbol symbol, const HashMap<id_t, Multiset> data)
        : history()
        , data(data)
        , mset()
        , symbol(symbol)
    {
    }

    AbstractSet project();
    void select(id_t key);
    void unselect();
    ENode make_enode();
    void reset();
};

} // namespace eqsat
