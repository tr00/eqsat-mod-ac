#pragma once

#include <cstddef>
#include <fstream>
#include <variant>

#include "handle.h"
#include "indices/abstract_index.h"
#include "relations/relation_ac.h"
#include "relations/row_store.h"
#include "symbol_table.h"

namespace eqsat
{

class AbstractRelation
{
  private:
    std::variant<RowStore, RelationAC> impl;

  public:
    explicit AbstractRelation(RowStore rel)
        : impl(std::move(rel))
    {
    }

    explicit AbstractRelation(RelationAC rel)
        : impl(std::move(rel))
    {
    }

    AbstractRelation(const AbstractRelation&) = default;
    AbstractRelation(AbstractRelation&&) = default;

    AbstractRelation& operator=(const AbstractRelation&) = delete;
    AbstractRelation& operator=(AbstractRelation&&) = delete;

    Symbol get_symbol() const
    {
        return std::visit([](const auto& rel) { return rel.get_symbol(); }, impl);
    }

    size_t size() const
    {
        return std::visit([](const auto& rel) { return rel.size(); }, impl);
    }

    void add_tuple(const Vec<id_t>& tuple)
    {
        std::visit([&tuple](auto& rel) { rel.add_tuple(tuple); }, impl);
    }

    AbstractIndex populate_index(uint32_t veo)
    {
        return std::visit([veo](auto& rel) { return rel.populate_index(veo); }, impl);
    }

    bool rebuild(Handle handle)
    {
        return std::visit([handle](auto& rel) { return rel.rebuild(handle); }, impl);
    }

    void dump(std::ofstream& out, const SymbolTable& symbols) const
    {
        return std::visit([&out, &symbols](auto& rel) { rel.dump(out, symbols); }, impl);
    }

    bool is_ac() const
    {
        return std::holds_alternative<RelationAC>(impl);
    }
};

} // namespace eqsat
