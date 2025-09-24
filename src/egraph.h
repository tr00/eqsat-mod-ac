#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include "id.h"
#include "symbol_table.h"
#include "theory.h"
#include "database.h"
#include "union_find.h"

class ENode {
private:
    symbol_t op;
    std::vector<id_t> children;

public:
    ENode(symbol_t op, std::vector<id_t> children) : op(op), children(std::move(children)) {}

    bool operator==(const ENode& other) const {
        return op == other.op && children == other.children;
    }

    symbol_t get_operator() const {
        return op;
    }

    const std::vector<id_t>& get_children() const {
        return children;
    }
};

namespace std {
    template<>
    struct hash<ENode> {
        size_t operator()(const ENode& node) const {
            size_t h1 = std::hash<symbol_t>{}(node.get_operator());
            size_t h2 = 0;

            auto children = node.get_children();
            for (const auto& child : children) {
                h2 ^= std::hash<id_t>{}(child) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
            }
            return h1 ^ (h2 << 1);
        }
    };
}

class EGraph {
private:
    Theory theory;
    Database db;
    UnionFind uf;
    std::unordered_map<ENode, id_t> memo;
    std::vector<id_t> worklist;

public:
    EGraph(const Theory& theory);
    ~EGraph() = default;

    EGraph(const EGraph&) = delete;
    EGraph& operator=(const EGraph&) = delete;

    EGraph(EGraph&&) = default;
    EGraph& operator=(EGraph&&) = default;

    id_t add_expr(std::shared_ptr<Expression> expression);
    id_t unify(id_t a, id_t b);
    bool is_equiv(id_t a, id_t b) { return uf.same(a, b); }
    void saturate(size_t max_iters);
};
