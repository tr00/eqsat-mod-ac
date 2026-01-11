#pragma once

#include <cstdint>

#include "ankerl/unordered_dense.h"
#include "gch/small_vector.hpp"
#include "utils/hash.h"

namespace eqsat
{

using id_t = uint32_t;

using Symbol = uint32_t;

/// CREDIT: https://github.com/gharveymn/small_vector
template <typename T, unsigned int InlineCapacity = gch::default_buffer_size<std::allocator<T>>::value>
using Vec = gch::small_vector<T, InlineCapacity>;

/// CREDIT: https://github.com/martinus/unordered_dense
template <typename K, typename V, typename Hash = ankerl::unordered_dense::hash<K>, typename Eq = std::equal_to<K>>
using HashMap = ankerl::unordered_dense::map<K, V, Hash, Eq>;

/// CREDIT: https://github.com/martinus/unordered_dense
template <typename K, typename Hash = ankerl::unordered_dense::hash<K>, typename Eq = std::equal_to<K>>
using HashSet = ankerl::unordered_dense::set<K, Hash, Eq>;

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

} // namespace eqsat

namespace std
{

template <>
struct hash<eqsat::ENode>
{
    size_t operator()(const eqsat::ENode& node) const
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
