#pragma once

#include <optional>

#include "types.h"

namespace eqsat
{

class EGraph;
class UnionFind;

// exposes only the enode lookup functionality
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

// exposes only the equivalence relation
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

} // namespace eqsat
