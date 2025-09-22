#pragma once

#include <vector>
#include "id.h"

class UnionFind {
private:
    std::vector<id_t> vec;

    id_t _find_root_ph(id_t x);
public:
    UnionFind();
    ~UnionFind() = default;

    UnionFind(const UnionFind&) = delete;
    UnionFind& operator=(const UnionFind&) = delete;

    UnionFind(UnionFind&&) = default;
    UnionFind& operator=(UnionFind&&) = default;

    id_t make_set();
    id_t unify(id_t a, id_t b);

    inline id_t find_root(id_t x) {
        // quick check which helps the branch predictor
        // since most of our data is already canonical
        if (vec[x] == x)
            return x;

        // iterative version with path halving
        return _find_root_ph(x);
    }

    inline bool same(id_t a, id_t b) {
        return find_root(a) == find_root(b);
    }

    std::size_t size() const { return vec.size(); }

    void normalize();
};
