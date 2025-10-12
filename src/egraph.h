#pragma once

#include <memory>

#include "database.h"
#include "id.h"
#include "query.h"
#include "symbol_table.h"
#include "theory.h"
#include "union_find.h"

class ENode
{
  public:
    Symbol op;
    const Vec<id_t> children;

    ENode(Symbol op, Vec<id_t> children) : op(op), children(children)
    {
    }

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
        size_t h1 = std::hash<Symbol>{}(node.op);
        size_t h2 = 0;

        auto children = node.children;
        for (const auto& child : children)
        {
            h2 ^= std::hash<id_t>{}(child) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
        }
        return h1 ^ (h2 << 1);
    }
};
} // namespace std

class EGraph
{
  private:
    Theory theory;
    Database db;
    UnionFind uf;
    HashMap<ENode, id_t> memo;

    Vec<id_t> worklist; // really needed?

    Vec<Query> queries;
    Vec<Subst> substs;

    int enodes = 0;

  public:
    EGraph(const Theory& theory);
    ~EGraph() = default;

    EGraph(const EGraph&) = delete;
    EGraph& operator=(const EGraph&) = delete;

    EGraph(EGraph&&) = default;
    EGraph& operator=(EGraph&&) = default;

    id_t add_expr(std::shared_ptr<Expr> expression);
    id_t add_enode(ENode enode);
    id_t add_enode(Symbol symbol, Vec<id_t> children);

    id_t unify(id_t a, id_t b);

    bool is_equiv(id_t a, id_t b)
    {
        return uf.same(a, b);
    }

    void apply_matches(const Vec<id_t>& match_vec, Subst& subst);

    void saturate(size_t max_iters);
};
