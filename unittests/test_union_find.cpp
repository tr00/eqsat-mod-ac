#include <catch2/catch_test_macros.hpp>

#include "union_find.h"

TEST_CASE("UnionFind basic operations", "[union_find]") {
    UnionFind uf;

    SECTION("make_set creates new sets") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();
        id_t id3 = uf.make_set();

        REQUIRE(id1 == 0);
        REQUIRE(id2 == 1);
        REQUIRE(id3 == 2);
        REQUIRE(uf.size() == 3);
    }

    SECTION("find_root returns correct root") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();

        REQUIRE(uf.find_root(id1) == id1);
        REQUIRE(uf.find_root(id2) == id2);
    }

    SECTION("same returns false for different sets") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();

        REQUIRE_FALSE(uf.same(id1, id2));
    }

    SECTION("same returns true for same element") {
        id_t id1 = uf.make_set();

        REQUIRE(uf.same(id1, id1));
    }
}

TEST_CASE("UnionFind unify operations", "[union_find]") {
    UnionFind uf;

    SECTION("unify two single-element sets") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();

        REQUIRE_FALSE(uf.same(id1, id2));

        id_t root = uf.unify(id1, id2);

        REQUIRE(uf.same(id1, id2));
        REQUIRE(uf.find_root(id1) == root);
        REQUIRE(uf.find_root(id2) == root);
    }

    SECTION("unify already unified sets") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();

        id_t root1 = uf.unify(id1, id2);
        id_t root2 = uf.unify(id1, id2);

        REQUIRE(root1 == root2);
        REQUIRE(uf.same(id1, id2));
    }

    SECTION("unify multiple sets") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();
        id_t id3 = uf.make_set();
        id_t id4 = uf.make_set();

        uf.unify(id1, id2);
        uf.unify(id3, id4);

        REQUIRE(uf.same(id1, id2));
        REQUIRE(uf.same(id3, id4));
        REQUIRE_FALSE(uf.same(id1, id3));
        REQUIRE_FALSE(uf.same(id2, id4));

        uf.unify(id1, id3);

        REQUIRE(uf.same(id1, id2));
        REQUIRE(uf.same(id1, id3));
        REQUIRE(uf.same(id1, id4));
        REQUIRE(uf.same(id2, id3));
        REQUIRE(uf.same(id2, id4));
        REQUIRE(uf.same(id3, id4));
    }
}

TEST_CASE("UnionFind path compression", "[union_find]") {
    UnionFind uf;

    SECTION("path compression reduces tree height") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();
        id_t id3 = uf.make_set();
        id_t id4 = uf.make_set();

        uf.unify(id1, id2);
        uf.unify(id2, id3);
        uf.unify(id3, id4);

        id_t root = uf.find_root(id1);

        REQUIRE(uf.find_root(id1) == root);
        REQUIRE(uf.find_root(id2) == root);
        REQUIRE(uf.find_root(id3) == root);
        REQUIRE(uf.find_root(id4) == root);

        REQUIRE(uf.same(id1, id4));
    }

    SECTION("find_root consistency") {
        id_t id1 = uf.make_set();
        id_t id2 = uf.make_set();
        id_t id3 = uf.make_set();

        uf.unify(id1, id2);
        uf.unify(id2, id3);

        id_t root = uf.find_root(id1);
        REQUIRE(uf.find_root(id1) == root);
        REQUIRE(uf.find_root(id2) == root);
        REQUIRE(uf.find_root(id3) == root);
    }

    SECTION("path compression maintains correctness") {
        const int n = 100;
        std::vector<id_t> ids;

        for (int i = 0; i < n; ++i) {
            ids.push_back(uf.make_set());
        }

        for (int i = 1; i < n; ++i) {
            uf.unify(ids[0], ids[i]);
        }

        id_t root = uf.find_root(ids[0]);
        for (int i = 0; i < n; ++i) {
            REQUIRE(uf.find_root(ids[i]) == root);
            REQUIRE(uf.same(ids[0], ids[i]));
        }
    }
}
