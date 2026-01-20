#include "sorted_vec_set.h"

namespace eqsat
{

bool SortedVecSet::insert(id_t id)
{
    auto it = std::lower_bound(data.begin(), data.end(), id);

    if (it != data.end() && *it == id)
        return false;

    data.insert(it, id);
    return true;
}

} // namespace eqsat
