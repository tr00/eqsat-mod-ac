#pragma once

#include <cstdint>
#include <vector>

using Variable = std::uint8_t;

using Index = std::uint8_t;

class Selection {
    Index idx;
    Variable var;
};

class State {
    std::vector<Selection> selections;

    /**
     * all indices which contain this variable
     * we will intersect these when entering this state
     */
    std::vector<Index> indices;
};

class Engine {
    std::vector<Variable> vars;
    std::vector<Index> indices;
    std::vector<State> states;
};
