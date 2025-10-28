#include <algorithm>
#include <cstdlib>

#include "handle.h"
#include "indices/abstract_index.h"
#include "indices/trie_index.h"
#include "permutation.h"
#include "relations/row_store.h"

AbstractIndex RowStore::populate_index(uint32_t vo)
{
    auto trie = std::make_shared<TrieNode>();

    // precompute permutation indices
    Vec<uint32_t> iota(arity);
    for (size_t i = 0; i < arity; ++i)
        iota[i] = static_cast<uint32_t>(i);

    auto permuted_indices = index_to_permutation(vo, iota);

    Vec<id_t> buffer(arity);
    const id_t *base = data.data();
    for (size_t i = 0; i < size(); ++i)
    {
        const id_t *tuple = base + i * arity;
        std::copy(tuple, tuple + arity, buffer.begin());
        apply_permutation(permuted_indices, buffer);
        trie->insert_path(buffer);
    }

    return AbstractIndex(TrieIndex(trie));
}

// Comparison context for qsort
struct TupleCompareContext
{
    size_t arity;
};

// qsort comparison function for tuples
// Compares first (arity - 1) elements, ignoring the last element (ID)
static int tuple_compare(const void *a, const void *b, void *context)
{
    const id_t *tuple1 = static_cast<const id_t *>(a);
    const id_t *tuple2 = static_cast<const id_t *>(b);
    const TupleCompareContext *ctx = static_cast<const TupleCompareContext *>(context);

    // Compare first (arity - 1) elements
    for (size_t i = 0; i < ctx->arity - 1; ++i)
    {
        if (tuple1[i] < tuple2[i]) return -1;
        if (tuple1[i] > tuple2[i]) return 1;
    }
    return 0; // Equal
}

bool RowStore::rebuild(Handle handle)
{
    for (auto& id : data)
        id = handle.canonicalize(id);

    if (arity <= 1) return false; // Nothing to rebuild if only ID column

    const size_t num_tuples = size();
    if (num_tuples <= 1) return false; // Nothing to compare

    bool did_something = false;

    // Sort tuples by first (arity - 1) elements using qsort_r
    TupleCompareContext ctx{arity};
    qsort_r(data.data(), num_tuples, arity * sizeof(id_t), tuple_compare, &ctx);

    // Find neighboring tuples with identical arguments but different IDs
    for (size_t i = 0; i + 1 < num_tuples; ++i)
    {
        id_t *tuple1 = &data[i * arity];
        id_t *tuple2 = &data[(i + 1) * arity];

        // check if first (arity - 1) elements are identical
        for (size_t j = 0; j < arity - 1; ++j)
            if (tuple1[j] != tuple2[j]) continue;

        id_t id1 = tuple1[arity - 1];
        id_t id2 = tuple2[arity - 1];

        if (id1 == id2) continue;

        // unify and update ids
        id_t newid = handle.unify(id1, id2);

        tuple1[arity - 1] = newid;
        tuple2[arity - 1] = newid;

        did_something = true;
    }

    return did_something;
}

void RowStore::dump(std::ofstream& out, const SymbolTable& symbols) const
{
    out << "---- " << symbols.get_string(symbol) << "(" << arity - 1 << ") with " << size() << " tuples ----"
        << std::endl;

    const id_t *base = data.data();
    for (size_t i = 0; i < size(); ++i)
    {
        const id_t *tuple = base + i * arity;
        auto id = tuple[arity - 1];
        out << "eclass-id: " << id;
        if (arity > 1) out << "  args: ";
        for (size_t j = 0; j < arity - 1; ++j)
        {
            out << tuple[j];
            if (j < arity - 2) out << ", ";
        }
        out << std::endl;
    }
    out << std::endl;
}
