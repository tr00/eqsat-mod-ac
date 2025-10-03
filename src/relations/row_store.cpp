#include <algorithm>

#include "indices/abstract_index.h"
#include "indices/trie_index.h"
#include "permutation.h"
#include "relations/row_store.h"

AbstractIndex RowStore::build_index(uint32_t vo)
{
    TrieNode trie;

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
        trie.insert_path(buffer);
    }

    return AbstractIndex(TrieIndex(std::move(trie)));
}
