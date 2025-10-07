#include <algorithm>
#include <cassert>

#include "trie_index.h"

int TrieNode::find_key_index(id_t key) const
{
    auto it = std::lower_bound(keys.begin(), keys.end(), key);
    if (it != keys.end() && *it == key)
    {
        return std::distance(keys.begin(), it);
    }
    return -1;
}

void TrieNode::insert_path(const Vec<id_t>& path)
{
    if (path.empty())
    {
        return;
    }

    TrieNode *current = this;

    for (id_t key : path)
    {
        int index = current->find_key_index(key);

        if (index == -1)
        {
            // Insert key and create new child
            auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);
            int insert_index = std::distance(current->keys.begin(), it);

            current->keys.insert(it, key);
            auto new_child = std::make_unique<TrieNode>();
            TrieNode *new_child_ptr = new_child.get();
            current->children.insert(current->children.begin() + insert_index, std::move(new_child));
            current = new_child_ptr;
        }
        else
        {
            current = current->children[index].get();
        }
    }
}

void TrieIndex::reset()
{
    current_node = root.get();
    parent_stack.clear();
}

void TrieIndex::select(id_t key)
{
    int index = current_node->find_key_index(key);
    assert(index != -1);

    parent_stack.push_back(current_node);
    current_node = current_node->children[index].get();
}

void TrieIndex::unselect()
{
    assert(!parent_stack.empty());

    current_node = parent_stack.back();
    parent_stack.pop_back();
}

AbstractSet TrieIndex::project() const
{
    return AbstractSet(SortedIterSet(current_node->keys));
}
