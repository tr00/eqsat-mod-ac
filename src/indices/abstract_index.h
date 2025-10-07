#pragma once

#include "trie_index.h"

enum IndexKind
{
    NONE,
    TRIE
};

class AbstractIndex
{
  private:
    IndexKind kind;
    union {
        TrieIndex trie;
    };

  public:
    // Default constructor creates a NONE index (no allocation)
    AbstractIndex() : kind(NONE)
    {
    }

    explicit AbstractIndex(TrieIndex index) : kind(TRIE), trie(std::move(index))
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
        }
    }

    // Copy constructor - allows independent traversal of the same underlying data
    AbstractIndex(const AbstractIndex& other) : kind(other.kind)
    {
        switch (kind)
        {
        case NONE:
            break;
        case TRIE:
            new (&trie) TrieIndex(other.trie);
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
            }
        }
        return *this;
    }

    // Move constructor
    AbstractIndex(AbstractIndex&& other) : kind(other.kind)
    {
        switch (kind)
        {
        case NONE:
            break;
        case TRIE:
            new (&trie) TrieIndex(std::move(other.trie));
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
        }
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
        }
    }
};
