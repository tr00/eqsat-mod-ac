#include <algorithm>
#include "union_find.h"

UnionFind::UnionFind() {}

id_t UnionFind::make_set() {
    vec.push_back(-1);
    return static_cast<id_t>(vec.size() - 1);
}

id_t UnionFind::find_root(id_t x) {
    while (vec[x] >= 0) {
        x = static_cast<id_t>(vec[x]);
    }
    return x;
}

id_t UnionFind::unify(id_t a, id_t b) {
    id_t ra = find_root(a);
    id_t rb = find_root(b);

    if (ra == rb) {
        return rb;
    }

    std::int32_t sa = -vec[ra];
    std::int32_t sb = -vec[rb];

    if (sa > sb) {
        std::swap(ra, rb);
        std::swap(sa, sb);
    }

    vec[rb] -= sa;
    vec[ra] = static_cast<std::int32_t>(rb);

    return rb;
}

bool UnionFind::same(id_t a, id_t b) {
    return find_root(a) == find_root(b);
}
