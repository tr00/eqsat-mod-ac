#pragma once

#include <cstdint>
#include <vector>
#include "id.h"

class UnionFind {
private:
    std::vector<std::int32_t> vec;

public:
    UnionFind();
    ~UnionFind() = default;

    UnionFind(const UnionFind&) = delete;
    UnionFind& operator=(const UnionFind&) = delete;

    UnionFind(UnionFind&&) = default;
    UnionFind& operator=(UnionFind&&) = default;

    id_t make_set();
    id_t find_root(id_t x);
    id_t unify(id_t a, id_t b);
    bool same(id_t a, id_t b);

    std::size_t size() const { return vec.size(); }
};
