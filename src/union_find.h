#pragma once

#include <fstream>

#include "types.h"

namespace eqsat
{

class UnionFind
{
  private:
    Vec<id_t> vec;
    size_t nclasses = 0;

    id_t _find_root_ph(id_t x);

  public:
    UnionFind();
    ~UnionFind() = default;

    UnionFind(const UnionFind&) = delete;
    UnionFind& operator=(const UnionFind&) = delete;

    UnionFind(UnionFind&&) = default;
    UnionFind& operator=(UnionFind&&) = default;

    size_t eclasses() const
    {
        return nclasses;
    }

    id_t make_set();
    id_t unify(id_t a, id_t b);

    inline id_t find_root(id_t x) noexcept
    {
        // quick check which helps the branch predictor
        // since most of our ids are already canonical
        if (vec[x] == x)
            return x;

        // iterative version with path halving
        return _find_root_ph(x);
    }

    inline id_t find_root(id_t x) const noexcept
    {
        while (vec[x] != x)
            x = vec[x];

        return x;
    }

    inline bool same(id_t a, id_t b) noexcept
    {
        return find_root(a) == find_root(b);
    }

    inline bool same(id_t a, id_t b) const noexcept
    {
        return find_root(a) == find_root(b);
    }

    std::size_t size() const noexcept
    {
        return vec.size();
    }

    void normalize();

    /**
     * @brief Dump all equivalence classes to a file
     *
     * Computes and outputs all equivalence classes in the union-find structure.
     * Each line shows a root ID followed by all IDs that belong to that equivalence class.
     *
     * @param out Output file stream
     *
     * @note This operation is expensive as it requires grouping all IDs by their roots
     */
    void dump_to_file(std::ofstream& out) const;
};

} // namespace eqsat
