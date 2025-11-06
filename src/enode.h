#pragma once

#include "id.h"
#include "symbol_table.h"
#include "utils/hash.h"
#include "utils/vec.h"

struct ENode
{
    Symbol op;
    Vec<id_t> children;

    ENode(Symbol op, Vec<id_t> children)
        : op(op)
        , children(children)
    {
    }

    ENode(const ENode&) = default;
    ENode(ENode&&) = default;
    ENode& operator=(const ENode&) = delete;
    ENode& operator=(ENode&&) = default;

    bool operator==(const ENode& other) const
    {
        return op == other.op && children == other.children;
    }
};

namespace std
{
template <>
struct hash<ENode>
{
    size_t operator()(const ENode& node) const
    {
        uint64_t h1 = eqsat::hash64(node.op);
        uint64_t h2 = eqsat::hash64(node.children.size());

        const auto& children = node.children;
        for (const auto& child : children)
            h1 = eqsat::mix64(h1, child);

        return eqsat::mix64(h1, h2);
    }
};
} // namespace std
