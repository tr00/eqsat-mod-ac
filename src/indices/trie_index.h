#pragma once

#include <memory>
#include <vector>

#include "id.h"
#include "../sets/abstract_set.h"

class TrieNode {
public:
    std::vector<id_t> keys;
    std::vector<std::shared_ptr<TrieNode>> children;

    TrieNode() = default;

    int find_key_index(id_t key) const;

    void insert_path(const std::vector<id_t> &path);
};

class TrieIndex {
private:
    TrieNode* current_node;
    std::vector<TrieNode *> parent_stack;

public:
    TrieIndex(TrieNode &trie);

    void select(id_t key);
    void backtrack();
    AbstractSet project() const;
};
