#pragma once

#include "enode.h"
#include "indices/multiset_index.h"
#include "trie_index.h"

enum IndexKind
{
    NONE,
    TRIE,
    MSET,
};

class AbstractIndex
{
  private:
    IndexKind kind;
    union {
        TrieIndex trie;
        MultisetIndex mst;
    };

  public:
    // Default constructor creates a NONE index (no allocation)
    AbstractIndex()
        : kind(NONE)
    {
    }

    explicit AbstractIndex(TrieIndex index)
        : kind(TRIE)
        , trie(std::move(index))
    {
    }

    explicit AbstractIndex(MultisetIndex index)
        : kind(MSET)
        , mst(std::move(index))
    {
    }

    ~AbstractIndex()
    {
        switch (kind)
        {
        case NONE:
            break;
        case TRIE:
            trie.~TrieIndex();
            break;
        case MSET:
            mst.~MultisetIndex();
            break;
        }
    }

    // Copy constructor - allows independent traversal of the same underlying data
    AbstractIndex(const AbstractIndex& other)
        : kind(other.kind)
    {
        switch (kind)
        {
        case NONE:
            break;
        case TRIE:
            new (&trie) TrieIndex(other.trie);
            break;
        case MSET:
            new (&mst) MultisetIndex(other.mst);
            break;
        }
    }

    // Copy assignment operator
    AbstractIndex& operator=(const AbstractIndex& other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case NONE:
                break;
            case TRIE:
                trie.~TrieIndex();
                break;
            case MSET:
                mst.~MultisetIndex();
                break;
            }
            // Copy construct new object
            kind = other.kind;
            switch (kind)
            {
            case NONE:
                break;
            case TRIE:
                new (&trie) TrieIndex(other.trie);
                break;
            case MSET:
                new (&mst) MultisetIndex(other.mst);
                break;
            }
        }
        return *this;
    }

    // Move constructor
    AbstractIndex(AbstractIndex&& other)
        : kind(other.kind)
    {
        switch (kind)
        {
        case NONE:
            break;
        case TRIE:
            new (&trie) TrieIndex(std::move(other.trie));
            break;
        case MSET:
            new (&mst) MultisetIndex(std::move(other.mst));
            break;
        }
    }

    // Move assignment operator
    AbstractIndex& operator=(AbstractIndex&& other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case NONE:
                break;
            case TRIE:
                trie.~TrieIndex();
                break;
            case MSET:
                mst.~MultisetIndex();
                break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind)
            {
            case NONE:
                break;
            case TRIE:
                new (&trie) TrieIndex(std::move(other.trie));
                break;
            case MSET:
                new (&mst) MultisetIndex(std::move(other.mst));
                break;
            }
        }
        return *this;
    }

    AbstractSet project()
    {
        switch (kind)
        {
        case NONE:
            assert(0 && "Cannot project from NONE index");
        case TRIE:
            return trie.project();
        case MSET:
            return mst.project();
        }
        assert(0);
    }

    void select(id_t key)
    {
        switch (kind)
        {
        case NONE:
            assert(0 && "Cannot select on NONE index");
            break;
        case TRIE:
            trie.select(key);
            break;
        case MSET:
            mst.select(key);
            break;
        }
    }

    void unselect()
    {
        switch (kind)
        {
        case NONE:
            assert(0 && "Cannot unselect on NONE index");
            break;
        case TRIE:
            trie.unselect();
            break;
        case MSET:
            mst.unselect();
            break;
        }
    }

    ENode make_enode()
    {
        switch (kind)
        {
        case NONE:
            assert(0 && "Cannot unselect on NONE index");
            break;
        case TRIE:
            // TODO:
            // trie.make_enode();
            break;
        case MSET:
            mst.make_enode();
            break;
        }

        __builtin_unreachable();
    }

    void reset()
    {
        switch (kind)
        {
        case NONE:
            assert(0 && "Cannot reset NONE index");
            break;
        case TRIE:
            trie.reset();
            break;
        case MSET:
            mst.reset();
            break;
        }
    }
};
