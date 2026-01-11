#include <algorithm>

#include "egraph.h"
#include "handle.h"

namespace eqsat
{

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

[[nodiscard]] bool Handle::equiv(id_t a, id_t b)
{
    return egraph.is_equiv(a, b);
}

[[nodiscard]] bool Handle::equiv(id_t a, id_t b) const
{
    return egraph.is_equiv(a, b);
}

id_t Handle::unify(id_t a, id_t b)
{
    return egraph.unify(a, b);
}

id_t Handle::add_enode(ENode enode)
{
    return egraph.add_enode(std::move(enode));
}

void Handle::add_enode_to_memo(id_t id, ENode enode)
{
    if (egraph.theory.get_arity(enode.op) == AC)
    {
        std::sort(enode.children.begin(), enode.children.end());
    }

    egraph.memo.emplace(std::move(enode), id);
}

} // namespace eqsat
