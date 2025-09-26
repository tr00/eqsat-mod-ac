#pragma once

#include <cassert>
#include <vector>

#include "../id.h"
#include "sorted_iter_set.h"
#include "sorted_set.h"

enum SetKind
{
    SORTED_VEC,
    SORTED_ITER,
};

class AbstractSet
{
  private:
    SetKind kind;
    union {
        SortedVecSet sorted_vec;
        SortedIterSet sorted_iter;
    };

  public:
    explicit AbstractSet(SortedVecSet s) : kind(SORTED_VEC), sorted_vec(std::move(s))
    {
    }
    explicit AbstractSet(SortedIterSet s) : kind(SORTED_ITER), sorted_iter(std::move(s))
    {
    }

    ~AbstractSet()
    {
        switch (kind)
        {
        case SORTED_VEC:
            sorted_vec.~SortedVecSet();
            break;
        case SORTED_ITER:
            sorted_iter.~SortedIterSet();
            break;
        }
    }

    AbstractSet(const AbstractSet &other) = delete;
    AbstractSet &operator=(const AbstractSet &other) = delete;

    AbstractSet(AbstractSet &&other) : kind(other.kind)
    {
        switch (kind)
        {
        case SORTED_VEC:
            new (&sorted_vec) SortedVecSet(std::move(other.sorted_vec));
            break;
        case SORTED_ITER:
            new (&sorted_iter) SortedIterSet(std::move(other.sorted_iter));
            break;
        }
    }

    AbstractSet &operator=(AbstractSet &&other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case SORTED_VEC:
                sorted_vec.~SortedVecSet();
                break;
            case SORTED_ITER:
                sorted_iter.~SortedIterSet();
                break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind)
            {
            case SORTED_VEC:
                new (&sorted_vec) SortedVecSet(std::move(other.sorted_vec));
                break;
            case SORTED_ITER:
                new (&sorted_iter) SortedIterSet(std::move(other.sorted_iter));
                break;
            }
        }
        return *this;
    }

    bool contains(id_t id) const
    {
        switch (kind)
        {
        case SORTED_VEC:
            return sorted_vec.contains(id);
        case SORTED_ITER:
            return sorted_iter.contains(id);
        }
        assert(0);
    }

    size_t size() const
    {
        switch (kind)
        {
        case SORTED_VEC:
            return sorted_vec.size();
        case SORTED_ITER:
            return sorted_iter.size();
        }
        assert(0);
    }

    bool empty() const
    {
        return size() == 0;
    }

    template <typename Func> void for_each(Func f) const
    {
        switch (kind)
        {
        case SORTED_VEC:
            return sorted_vec.for_each(f);
        case SORTED_ITER:
            return sorted_iter.for_each(f);
        }
        assert(0);
    }
};

void intersect_many(SortedVecSet &output, const std::vector<AbstractSet> &sets);
