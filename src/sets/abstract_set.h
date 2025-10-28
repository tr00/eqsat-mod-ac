#pragma once

#include <cassert>
#include <functional>

#include "../id.h"
#include "sets/hashmap_wrapper.h"
#include "sets/multiset_support.h"
#include "sets/singleton_set.h"
#include "sorted_iter_set.h"
#include "sorted_vec_set.h"

enum SetKind
{
    EMPTY,
    SORTED_VEC,
    SORTED_ITER,
    MSET_SUPPORT,
    HMAP_WRAPPER,
    SINGLETON,
};

class AbstractSet
{
  private:
    SetKind kind;
    union {
        SortedVecSet sorted_vec;
        SortedIterSet sorted_iter;
        MultisetSupport mset_support;
        WrappedHashMapSet hmap_wrapper;
        SingletonSet singleton;
    };

  public:
    AbstractSet()
        : kind(EMPTY)
    {
    }
    explicit AbstractSet(SortedVecSet s)
        : kind(SORTED_VEC)
        , sorted_vec(std::move(s))
    {
    }
    explicit AbstractSet(SortedIterSet s)
        : kind(SORTED_ITER)
        , sorted_iter(std::move(s))
    {
    }
    explicit AbstractSet(MultisetSupport s)
        : kind(MSET_SUPPORT)
        , mset_support(std::move(s))
    {
    }
    explicit AbstractSet(WrappedHashMapSet s)
        : kind(HMAP_WRAPPER)
        , hmap_wrapper(std::move(s))
    {
    }
    explicit AbstractSet(SingletonSet s)
        : kind(SINGLETON)
        , singleton(std::move(s))
    {
    }

    ~AbstractSet()
    {
        switch (kind)
        {
        case EMPTY:
            break;
        case SORTED_VEC:
            sorted_vec.~SortedVecSet();
            break;
        case SORTED_ITER:
            sorted_iter.~SortedIterSet();
            break;
        case MSET_SUPPORT:
            mset_support.~MultisetSupport();
            break;
        case HMAP_WRAPPER:
            hmap_wrapper.~WrappedHashMapSet();
            break;
        case SINGLETON:
            singleton.~SingletonSet();
            break;
        }
    }

    AbstractSet(const AbstractSet& other) = delete;
    AbstractSet& operator=(const AbstractSet& other) = delete;

    AbstractSet(AbstractSet&& other)
        : kind(other.kind)
    {
        switch (kind)
        {
        case EMPTY:
            break;
        case SORTED_VEC:
            new (&sorted_vec) SortedVecSet(std::move(other.sorted_vec));
            break;
        case SORTED_ITER:
            new (&sorted_iter) SortedIterSet(std::move(other.sorted_iter));
            break;
        case MSET_SUPPORT:
            new (&mset_support) MultisetSupport(std::move(other.mset_support));
            break;
        case HMAP_WRAPPER:
            new (&hmap_wrapper) WrappedHashMapSet(std::move(other.hmap_wrapper));
            break;
        case SINGLETON:
            new (&singleton) SingletonSet(std::move(other.singleton));
            break;
        }
    }

    AbstractSet& operator=(AbstractSet&& other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case EMPTY:
                break;
            case SORTED_VEC:
                sorted_vec.~SortedVecSet();
                break;
            case SORTED_ITER:
                sorted_iter.~SortedIterSet();
                break;
            case MSET_SUPPORT:
                mset_support.~MultisetSupport();
                break;
            case HMAP_WRAPPER:
                hmap_wrapper.~WrappedHashMapSet();
                break;
            case SINGLETON:
                singleton.~SingletonSet();
                break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind)
            {
            case EMPTY:
                break;
            case SORTED_VEC:
                new (&sorted_vec) SortedVecSet(std::move(other.sorted_vec));
                break;
            case SORTED_ITER:
                new (&sorted_iter) SortedIterSet(std::move(other.sorted_iter));
                break;
            case MSET_SUPPORT:
                new (&mset_support) MultisetSupport(std::move(other.mset_support));
                break;
            case HMAP_WRAPPER:
                new (&hmap_wrapper) WrappedHashMapSet(std::move(other.hmap_wrapper));
                break;
            case SINGLETON:
                new (&singleton) SingletonSet(std::move(other.singleton));
                break;
            }
        }
        return *this;
    }

    bool contains(id_t id) const
    {
        switch (kind)
        {
        case EMPTY:
            return false;
        case SORTED_VEC:
            return sorted_vec.contains(id);
        case SORTED_ITER:
            return sorted_iter.contains(id);
        case MSET_SUPPORT:
            return mset_support.contains(id);
        case HMAP_WRAPPER:
            return hmap_wrapper.contains(id);
        case SINGLETON:
            return singleton.contains(id);
        }
        assert(0);
    }

    size_t size() const
    {
        switch (kind)
        {
        case EMPTY:
            return 0;
        case SORTED_VEC:
            return sorted_vec.size();
        case SORTED_ITER:
            return sorted_iter.size();
        case MSET_SUPPORT:
            return mset_support.size();
        case HMAP_WRAPPER:
            return hmap_wrapper.size();
        case SINGLETON:
            return singleton.size();
        }
        assert(0);
    }

    bool empty() const
    {
        return size() == 0;
    }

    void for_each(std::function<void(id_t)> f) const
    {
        switch (kind)
        {
        case EMPTY:
            return;
        case SORTED_VEC:
            return sorted_vec.for_each(f);
        case SORTED_ITER:
            return sorted_iter.for_each(f);
        case MSET_SUPPORT:
            return mset_support.for_each(f);
        case HMAP_WRAPPER:
            return hmap_wrapper.for_each(f);
        case SINGLETON:
            return singleton.for_each(f);
        }
        assert(0);
    }
};

size_t intersect_many(SortedVecSet& output, const Vec<AbstractSet>& sets);
