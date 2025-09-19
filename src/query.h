#pragma once

#include <vector>
#include "symbol_table.h"

// Variable type for query variables
using var_t = uint32_t;

// A constraint consists of an operator and a list of variables
class Constraint {
public:
    symbol_t operator_symbol;
    std::vector<var_t> variables;

    Constraint(symbol_t op, const std::vector<var_t>& vars);
};

// A conjunctive query consists of constraints and a head
class Query {
public:
    std::vector<Constraint> constraints;
    std::vector<var_t> head;

    Query();
    Query(const std::vector<Constraint>& constraints, const std::vector<var_t>& head);

    void add_constraint(const Constraint& constraint);
    void add_constraint(symbol_t op, const std::vector<var_t>& vars);
    void add_head_var(var_t var);
};
