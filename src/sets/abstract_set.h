#pragma once

#include <cassert>
#include <functional>
#include <variant>

#include "sets/hashmap_wrapper.h"
#include "sets/multiset_support.h"
#include "sets/singleton_set.h"
#include "sets/sorted_iter_set.h"
#include "sets/sorted_vec_set.h"
#include "types.h"

namespace eqsat
{

struct EmptySet
{
    bool contains(id_t) const
    {
        return false;
    }

    size_t size() const
    {
        return 0;
    }

    void for_each(std::function<void(id_t)>) const
    {
    }
};

class AbstractSet
{
  private:
    std::variant<EmptySet, SortedVecSet, SortedIterSet, MultisetSupport, WrappedHashMapSet, SingletonSet> impl;

  public:
    AbstractSet()
        : impl(EmptySet{})
    {
    }
    explicit AbstractSet(SortedVecSet s)
        : impl(std::move(s))
    {
    }
    explicit AbstractSet(SortedIterSet s)
        : impl(std::move(s))
    {
    }
    explicit AbstractSet(MultisetSupport s)
        : impl(std::move(s))
    {
    }
    explicit AbstractSet(WrappedHashMapSet s)
        : impl(std::move(s))
    {
    }
    explicit AbstractSet(SingletonSet s)
        : impl(std::move(s))
    {
    }

    AbstractSet(AbstractSet&) = default;
    AbstractSet(AbstractSet&&) = default;

    AbstractSet& operator=(AbstractSet&) = delete;
    AbstractSet& operator=(AbstractSet&&) = delete;

    bool contains(id_t id) const
    {
        return std::visit([id](const auto& set) { return set.contains(id); }, impl);
    }

    size_t size() const
    {
        return std::visit([](const auto& set) { return set.size(); }, impl);
    }

    bool empty() const
    {
        return size() == 0;
    }

    void for_each(std::function<void(id_t)> f) const
    {
        std::visit([&f](const auto& set) { set.for_each(f); }, impl);
    }
};

size_t intersect_many(SortedVecSet& output, const Vec<AbstractSet>& sets);

} // namespace eqsat
