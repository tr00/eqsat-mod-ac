/// Implementation of the union-find data structure
/// It does perform path splitting but not union-by-{rank,size}
/// when unifying two sets it chooses the one with the smaller id.

#include <algorithm>
#include <fstream>

#include "union_find.h"
#include "utils/hashmap.h"

UnionFind::UnionFind()
{
}

id_t UnionFind::make_set()
{
    ++nclasses;
    id_t x = static_cast<id_t>(vec.size());
    vec.push_back(x);
    return x;
}

id_t UnionFind::_find_root_ph(id_t x)
{
    while (vec[x] != x)
    {
        vec[x] = vec[vec[x]];
        x = vec[x];
    }

    return x;
}

id_t UnionFind::unify(id_t a, id_t b)
{
    id_t root_a = find_root(a);
    id_t root_b = find_root(b);

    if (root_a == root_b)
        return root_a;

    if (root_a > root_b)
        std::swap(root_a, root_b);

    vec[root_b] = root_a;

    nclasses--;
    return root_a;
}

void UnionFind::normalize()
{
    for (size_t i = 0; i < vec.size(); ++i)
        vec[i] = vec[vec[i]];
}

void UnionFind::dump_to_file(std::ofstream& out) const
{
    out << "====<< Union-Find >>====\n\n";

    // root --> members
    HashMap<id_t, Vec<id_t>> classes;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        id_t root = find_root(static_cast<id_t>(i));
        classes[root].push_back(static_cast<id_t>(i));
    }

    Vec<id_t> roots;
    roots.reserve(classes.size());
    for (const auto& [root, _] : classes)
        roots.push_back(root);
    std::sort(roots.begin(), roots.end());

    for (id_t root : roots)
    {
        const auto& members = classes[root];
        out << "  {";
        for (size_t i = 0; i < members.size(); ++i)
        {
            out << members[i];
            if (i < members.size() - 1)
                out << ", ";
        }
        out << "}\n";
    }
    out << "\n";
}
