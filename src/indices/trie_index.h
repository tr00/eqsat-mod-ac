#pragma once

#include <memory>

#include "sets/abstract_set.h"
#include "types.h"

namespace eqsat
{

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
    Symbol symbol;
    Vec<id_t> history;

  public:
    TrieIndex(Symbol symbol, std::shared_ptr<TrieNode> root)
        : root(root)
        , symbol(symbol)
        , history()
    {
        reset();
    }

    // Make TrieIndex copyable - each copy can traverse independently
    TrieIndex(const TrieIndex& other)
        : current_node(other.current_node)
        , parent_stack(other.parent_stack)
        , root(other.root)
        , symbol(other.symbol)
        , history(other.history)
    {
    }

    TrieIndex& operator=(const TrieIndex& other)
    {
        root = other.root;
        current_node = other.current_node;
        parent_stack = other.parent_stack;
        history = other.history;
        symbol = other.symbol;
        return *this;
    }

    void reset();
    void select(id_t key);
    void unselect();
    AbstractSet project() const;
    ENode make_enode() const;
};

} // namespace eqsat
