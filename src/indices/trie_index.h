#pragma once

#include <memory>

#include "../sets/abstract_set.h"
#include "id.h"

class TrieNode
{
  public:
    Vec<id_t> keys;
    Vec<std::shared_ptr<TrieNode>> children;

    TrieNode() = default;

    int find_key_index(id_t key) const;

    void insert_path(const Vec<id_t>& path);
};

class TrieIndex
{
  private:
    TrieNode *current_node;
    Vec<TrieNode *> parent_stack;
    std::shared_ptr<TrieNode> root;

  public:
    TrieIndex(std::shared_ptr<TrieNode> root)
        : root(root)
    {
        reset();
    }

    // Make TrieIndex copyable - each copy can traverse independently
    TrieIndex(const TrieIndex& other)
        : root(other.root)
        , current_node(other.current_node)
        , parent_stack(other.parent_stack)
    {
    }

    TrieIndex& operator=(const TrieIndex& other)
    {
        root = other.root;
        current_node = other.current_node;
        parent_stack = other.parent_stack;
        return *this;
    }

    void reset();
    void select(id_t key);
    void unselect();
    AbstractSet project() const;
};
