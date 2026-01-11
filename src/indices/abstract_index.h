#pragma once

#include <variant>

#include "indices/multiset_index.h"
#include "indices/trie_index.h"
#include "types.h"

namespace eqsat
{

struct NullIndex
{
    AbstractSet project()
    {
        assert(0);
        __builtin_unreachable();
    }

    void select(id_t)
    {
        assert(0);
        __builtin_unreachable();
    }

    void unselect()
    {
        assert(0);
        __builtin_unreachable();
    }

    ENode make_enode()
    {
        assert(0);
        __builtin_unreachable();
    }

    void reset()
    {
    }
};

class AbstractIndex
{
  private:
    std::variant<NullIndex, TrieIndex, MultisetIndex> impl;

  public:
    AbstractIndex()
        : impl(NullIndex{})
    {
    }

    explicit AbstractIndex(TrieIndex index)
        : impl(std::move(index))
    {
    }

    explicit AbstractIndex(MultisetIndex index)
        : impl(std::move(index))
    {
    }

    AbstractIndex(const AbstractIndex& other) = default; // Copy constructor
    AbstractIndex(AbstractIndex&& other) = default;      // Move constructor

    AbstractIndex& operator=(const AbstractIndex& other) = delete; // Copy assignment operator
    AbstractIndex& operator=(AbstractIndex&& other) = default;     // Move assignment operator

    AbstractSet project()
    {
        return std::visit([](auto& index) { return index.project(); }, impl);
    }

    void select(id_t key)
    {
        std::visit([key](auto& index) { return index.select(key); }, impl);
    }

    void unselect()
    {
        std::visit([](auto& index) { return index.unselect(); }, impl);
    }

    ENode make_enode()
    {
        return std::visit([](auto& index) { return index.make_enode(); }, impl);
    }

    void reset()
    {
        std::visit([](auto& index) { return index.reset(); }, impl);
    }
};

} // namespace eqsat
