#pragma once

#include <optional>

#include "types.h"

namespace eqsat
{

// The idea here is that an e-graph is a very convoluted structure
// which is made up of three major components,
// and these components are independent in terms of storage,
// but very intertwined via invariants and algorithms on the e-graph level.
//
// We can split the parts' storage from the other parts' logic by dependency inversion.
// That is, a part can use some other part of the system with the established interface,
// having e.g. the circular dependency: egraph --> database --> unionfind --> egraph

class EGraph;

class EGraphLookupDI
{

  private:
    EGraph& egraph;

  protected:
    EGraphLookupDI(EGraph& egraph)
        : egraph(egraph)
    {
    }

    std::optional<id_t> lookup(ENode) const;
    id_t lookup_or_ephemeral(ENode);
};

class EGraphEquivalenceDI
{
  private:
    EGraph& egraph;

  protected:
    EGraphEquivalenceDI(EGraph& egraph)
        : egraph(egraph)
    {
    }

    id_t canonicalize(id_t) const;
    id_t canonicalize_mut(id_t); // with path compression

    id_t unify(id_t, id_t);

    bool is_equiv(id_t, id_t);
};

class EGraphTermBankDI
{
  private:
    EGraph& egraph;

  protected:
    EGraphTermBankDI(EGraph& egraph)
        : egraph(egraph)
    {
    }

    id_t add_enode(ENode);
    id_t add_enode(Symbol, Vec<id_t>);
};

class EGraphTheoryDI
{
  private:
    EGraph& egraph;

  protected:
    EGraphTheoryDI(EGraph& egraph)
        : egraph(egraph)
    {
    }

    bool is_ac(Symbol f);
};

} // namespace eqsat
