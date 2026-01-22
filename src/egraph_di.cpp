#include "egraph_di.h"
#include "egraph.h"

namespace eqsat
{

std::optional<id_t> EGraphLookupDI::lookup(ENode enode) const
{
    return egraph.lookup(enode);
}

id_t EGraphLookupDI::lookup_or_ephemeral(ENode enode)
{
    auto res = egraph.lookup(enode);

    if (res.has_value())
        return res.value();

    // During pattern matching, we want to know the id of an implicitly stored enode.
    // However, since it is only implicit it doesnt have an assigned id, so instead
    // we temporarily give it an ephemeral id which we remark in the msb of the id.
    // In case this term is part of a full match we instantiate the term during apply
    // and make it explicitly represented and assign it a new proper id.

    id_t id = static_cast<id_t>(egraph.ephemeral_map.size()) | 0x80000000;
    egraph.ephemeral_map.emplace(id, std::move(enode));

    return id;
}

id_t EGraphEquivalenceDI::canonicalize(id_t id) const
{
    return egraph.canonicalize(id);
}

id_t EGraphEquivalenceDI::canonicalize_mut(id_t id)
{
    return egraph.canonicalize(id);
}

} // namespace eqsat
