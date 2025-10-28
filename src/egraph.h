#pragma once

#include <memory>

#include "database.h"
#include "enode.h"
#include "handle.h"
#include "id.h"
#include "query.h"
#include "symbol_table.h"
#include "theory.h"
#include "union_find.h"

class EGraph
{
  private:
    Theory theory;
    Database db;
    UnionFind uf;
    HashMap<ENode, id_t> memo;

    Vec<Query> queries;
    Vec<Subst> substs;
    Vec<std::pair<Symbol, uint32_t>> required_indices;

    int enodes = 0;

    Handle handle()
    {
        return Handle(*this);
    }

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

    std::optional<id_t> lookup(ENode enode) const;

    id_t unify(id_t a, id_t b);

    bool is_equiv(id_t a, id_t b) const
    {
        return uf.same(a, b);
    }

    id_t canonicalize(id_t id)
    {
        return uf.find_root(id);
    }

    id_t canonicalize(id_t id) const
    {
        return uf.find_root(id);
    }

    void apply_matches(const Vec<id_t>& matches, Subst& subst);
    void apply_match(const Vec<id_t>& match, Subst& subst);
    bool rebuild();

    void saturate(size_t max_iters);

    void dump_to_file() const;
};
