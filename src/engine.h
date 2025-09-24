#pragma once

#include "sorted_set.h"
#include "trie_index.h"
#include <cstdint>
#include <vector>

using Variable = std::uint8_t;

using Index = std::uint8_t;

class Selection {
    Index idx;
    Variable var;
};

class Engine;

class State {
public:
    std::vector<Selection> selections;

    /**
     * all indices which contain this variable
     * we will intersect these when entering this state
     */
    std::vector<Index> indices;

    SortedSet candidates;
};

class Engine {
    std::vector<Variable> vars;

    // register file refering to the indices
    std::vector<TrieIndex> indices;
    std::vector<State> states;

    void run();
};
