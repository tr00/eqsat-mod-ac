#include "handle.h"
#include "egraph.h"

[[nodiscard]] std::optional<id_t> Handle::lookup(ENode enode) const
{
    return egraph.lookup(enode);
}

[[nodiscard]] id_t Handle::canonicalize(id_t id)
{
    return egraph.canonicalize(id);
}

[[nodiscard]] id_t Handle::canonicalize(id_t id) const
{
    return egraph.canonicalize(id);
}

id_t Handle::unify(id_t a, id_t b)
{
    return egraph.unify(a, b);
}

id_t Handle::add_enode(ENode enode)
{
    return egraph.add_enode(std::move(enode));
}
