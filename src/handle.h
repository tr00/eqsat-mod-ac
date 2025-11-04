#pragma once

#include "enode.h"

class EGraph;

class Handle
{
  private:
    EGraph& egraph;

  public:
    Handle(EGraph& egraph)
        : egraph(egraph)
    {
    }

    std::optional<id_t> lookup(ENode enode) const;

    id_t canonicalize(id_t);
    id_t canonicalize(id_t) const;

    id_t unify(id_t, id_t);

    id_t add_enode(ENode enode);
    void add_enode_to_memo(id_t id, ENode enode);
};
