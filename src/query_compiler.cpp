
#include "query_compiler.h"
#include "permutation.h"
#include "query.h"
#include <vector>

void QueryCompiler::build_indices(Query &query)
{
    std::vector<int> perm;

    for (auto &constr : query.constraints)
    {
        // Variable order is implicit in the naming of variables.
        // The var 0 is the first one that's being elaborated.
        // Hence, the constraint is already the permutation of attributes.
        uint32_t perm_idx = permutation_to_index(constr.variables);

        db.add_index(constr.operator_symbol, perm_idx);
    }
}
