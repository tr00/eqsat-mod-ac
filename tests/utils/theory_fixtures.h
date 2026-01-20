#pragma once

#include "theory.h"

namespace eqsat::test
{

Theory AbelianGroup(std::vector<std::string> vars)
{
    Theory theory;

    for (auto var : vars)
        theory.add_operator(var, 0);

    theory.add_operator("1", 0);
    theory.add_operator("inv", 1);
    theory.add_operator("mul", AC);

    return theory;
}

Theory CommutativeRing(std::vector<std::string> vars)
{
    Theory theory;

    for (auto var : vars)
        theory.add_operator(var, 0);

    theory.add_operator("0", 0);
    theory.add_operator("1", 0);
    theory.add_operator("+", AC);
    theory.add_operator("*", AC);

    return theory;
}

} // namespace eqsat::test
