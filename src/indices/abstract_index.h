#pragma once

#include "trie_index.h"

enum IndexKind {
    TRIE
};

class AbstractIndex {
private:
    IndexKind kind;
    union {
        TrieIndex trie;
    };

public:
    explicit AbstractIndex(TrieIndex index) : kind(TRIE), trie(std::move(index)) {}

    ~AbstractIndex() {
        switch (kind) {
            case TRIE: trie.~TrieIndex(); break;
        }
    }

    // Delete copy constructor and assignment operator
    AbstractIndex(const AbstractIndex&) = delete;
    AbstractIndex& operator=(const AbstractIndex&) = delete;

    // Move constructor
    AbstractIndex(AbstractIndex&& other) : kind(other.kind) {
        switch (kind) {
            case TRIE: new(&trie) TrieIndex(std::move(other.trie)); break;
        }
    }

    // Move assignment operator
    AbstractIndex& operator=(AbstractIndex&& other) {
        if (this != &other) {
            // Destroy current object
            switch (kind) {
                case TRIE: trie.~TrieIndex(); break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind) {
                case TRIE: new(&trie) TrieIndex(std::move(other.trie)); break;
            }
        }
        return *this;
    }

    AbstractSet project() {
        switch (kind) {
            case TRIE: return trie.project();
        }
        assert(0);
    }

    void select(id_t key);
    void backtrack();
};
