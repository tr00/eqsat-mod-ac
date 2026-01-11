#include <algorithm>
#include <cstdlib>

#include "handle.h"
#include "indices/abstract_index.h"
#include "indices/trie_index.h"
#include "permutation.h"
#include "relations/row_store.h"

namespace eqsat
{

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

    return AbstractIndex(TrieIndex(symbol, trie));
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
        if (tuple1[i] < tuple2[i])
            return -1;
        if (tuple1[i] > tuple2[i])
            return 1;
    }
    return 0; // Equal
}

void RowStore::deduplicate()
{
    if (data.empty())
        return;

    const size_t num_tuples = size();
    if (num_tuples <= 1)
        return;

    size_t write_idx = 0;
    size_t read_idx = arity;

    // Keep first tuple, compare rest
    while (read_idx < data.size())
    {
        const id_t *current = &data[write_idx];
        const id_t *candidate = &data[read_idx];

        // Check if all arity elements are identical
        bool is_duplicate = true;
        for (size_t i = 0; i < arity; ++i)
        {
            if (current[i] != candidate[i])
            {
                is_duplicate = false;
                break;
            }
        }

        if (!is_duplicate)
        {
            // Move to next write position and copy tuple
            write_idx += arity;
            if (write_idx != read_idx)
            {
                for (size_t i = 0; i < arity; ++i)
                    data[write_idx + i] = candidate[i];
            }
        }

        read_idx += arity;
    }

    // Resize to remove duplicates
    data.resize(write_idx + arity);
}

bool RowStore::rebuild(Handle handle)
{
    for (auto& id : data)
        id = handle.canonicalize(id);

    if (arity <= 1)
        return false; // Nothing to rebuild if only ID column

    const size_t num_tuples = size();
    if (num_tuples <= 1)
        return false; // Nothing to compare

    bool did_something = false;

    // Sort tuples by first (arity - 1) elements using qsort_r
    TupleCompareContext ctx{arity};
    qsort_r(data.data(), num_tuples, arity * sizeof(id_t), tuple_compare, &ctx);

    // Find neighboring tuples with identical arguments but different IDs
    for (size_t i = 0; i + 1 < num_tuples; ++i)
    {
        id_t id1, id2, newid;
        id_t *tuple1 = &data[i * arity];
        id_t *tuple2 = &data[(i + 1) * arity];

        for (size_t j = 0; j < arity - 1; ++j)
            if (tuple1[j] != tuple2[j])
                goto next_tuple;

        id1 = tuple1[arity - 1];
        id2 = tuple2[arity - 1];

        if (id1 == id2)
            continue;

        newid = handle.unify(id1, id2);

        tuple1[arity - 1] = newid;
        tuple2[arity - 1] = newid;

        did_something = true;

    next_tuple:;
    }

    // Remove duplicate tuples created by unification
    deduplicate();

    return did_something;
}

void RowStore::dump(std::ofstream& out, const SymbolTable& symbols) const
{
    out << "---- " << symbols.get_string(symbol) << "(" << arity - 1 << ") with " << size() << " tuples ----\n";

    const id_t *base = data.data();
    for (size_t i = 0; i < size(); ++i)
    {
        const id_t *tuple = base + i * arity;
        auto id = tuple[arity - 1];
        out << "eclass-id: " << id;
        if (arity > 1)
            out << "  args: ";
        for (size_t j = 0; j < arity - 1; ++j)
        {
            out << tuple[j];
            if (j < arity - 2)
                out << ", ";
        }
        out << std::endl;
    }
    out << std::endl;
}

} // namespace eqsat
