#include <catch2/catch_test_macros.hpp>

#include "utils/multiset.h"

using namespace eqsat;

TEST_CASE("Multiset basic operations", "[multiset]")
{
    Multiset ms;

    SECTION("Empty multiset")
    {
        REQUIRE(ms.empty());
        REQUIRE(ms.size() == 0);
        REQUIRE_FALSE(ms.contains(1));
        REQUIRE(ms.count(1) == 0);
    }

    SECTION("Insert single element")
    {
        ms.insert(10);

        REQUIRE_FALSE(ms.empty());
        REQUIRE(ms.size() == 1);
        REQUIRE(ms.contains(10));
        REQUIRE(ms.count(10) == 1);
        REQUIRE_FALSE(ms.contains(11));
    }

    SECTION("Insert multiple different elements")
    {
        ms.insert(10);
        ms.insert(20);
        ms.insert(15);

        REQUIRE(ms.size() == 3);
        REQUIRE(ms.contains(10));
        REQUIRE(ms.contains(15));
        REQUIRE(ms.contains(20));
        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.count(15) == 1);
        REQUIRE(ms.count(20) == 1);
    }

    SECTION("Insert same element multiple times")
    {
        ms.insert(10);
        ms.insert(10);
        ms.insert(10);

        REQUIRE(ms.size() == 3); // Total count with multiplicities
        REQUIRE(ms.contains(10));
        REQUIRE(ms.count(10) == 3);
    }

    SECTION("Insert maintains sorted order")
    {
        ms.insert(30);
        ms.insert(10);
        ms.insert(20);

        // Elements should be in sorted order internally
        REQUIRE(ms.contains(10));
        REQUIRE(ms.contains(20));
        REQUIRE(ms.contains(30));
    }
}

TEST_CASE("Multiset remove operations", "[multiset]")
{
    Multiset ms;

    SECTION("Remove from empty multiset does nothing")
    {
        ms.remove(10);

        REQUIRE(ms.empty());
        REQUIRE(ms.size() == 0);
    }

    SECTION("Remove non-existent element does nothing")
    {
        ms.insert(10);
        ms.remove(20);

        REQUIRE(ms.size() == 1);
        REQUIRE(ms.contains(10));
        REQUIRE(ms.count(10) == 1);
    }

    SECTION("Remove decrements count")
    {
        ms.insert(10);
        ms.insert(10);
        ms.insert(10);

        REQUIRE(ms.count(10) == 3);

        ms.remove(10);
        REQUIRE(ms.count(10) == 2);
        REQUIRE(ms.contains(10));

        ms.remove(10);
        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.contains(10));
    }

    SECTION("Remove to zero keeps pair")
    {
        ms.insert(10);
        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.contains(10));

        ms.remove(10);
        REQUIRE(ms.count(10) == 0);
        REQUIRE_FALSE(ms.contains(10));
        REQUIRE(ms.size() == 0); // No elements, count is zero

        // Can insert again
        ms.insert(10);
        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.contains(10));
        REQUIRE(ms.size() == 1);
    }

    SECTION("Remove below zero does nothing")
    {
        ms.insert(10);
        ms.remove(10);
        REQUIRE(ms.count(10) == 0);

        ms.remove(10);
        REQUIRE(ms.count(10) == 0);
        REQUIRE(ms.size() == 0);
    }

    SECTION("Remove with multiple elements")
    {
        ms.insert(10);
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);
        ms.insert(30);

        ms.remove(10);
        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.count(20) == 1);
        REQUIRE(ms.count(30) == 2);

        ms.remove(20);
        REQUIRE(ms.count(20) == 0);
        REQUIRE_FALSE(ms.contains(20));
        REQUIRE(ms.size() == 3); // 1 + 0 + 2 = 3 total elements
    }
}

TEST_CASE("Multiset clear operation", "[multiset]")
{
    Multiset ms;

    SECTION("Clear empty multiset")
    {
        ms.clear();
        REQUIRE(ms.empty());
        REQUIRE(ms.size() == 0);
    }

    SECTION("Clear non-empty multiset")
    {
        ms.insert(10);
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);

        REQUIRE(ms.size() == 4); // 2 + 1 + 1 = 4 total elements

        ms.clear();
        REQUIRE(ms.empty());
        REQUIRE(ms.size() == 0);
        REQUIRE_FALSE(ms.contains(10));
        REQUIRE_FALSE(ms.contains(20));
        REQUIRE_FALSE(ms.contains(30));
    }

    SECTION("Clear and reuse")
    {
        ms.insert(10);
        ms.clear();

        ms.insert(20);
        REQUIRE(ms.size() == 1);
        REQUIRE(ms.contains(20));
        REQUIRE_FALSE(ms.contains(10));
    }
}

TEST_CASE("Multiset edge cases", "[multiset]")
{
    Multiset ms;

    SECTION("Large counts")
    {
        for (int i = 0; i < 1000; ++i)
        {
            ms.insert(42);
        }

        REQUIRE(ms.count(42) == 1000);
        REQUIRE(ms.size() == 1000); // Total count with multiplicities

        for (int i = 0; i < 500; ++i)
        {
            ms.remove(42);
        }

        REQUIRE(ms.count(42) == 500);
        REQUIRE(ms.contains(42));
    }

    SECTION("Many unique elements")
    {
        for (id_t i = 0; i < 100; ++i)
        {
            ms.insert(i);
        }

        REQUIRE(ms.size() == 100);

        for (id_t i = 0; i < 100; ++i)
        {
            REQUIRE(ms.contains(i));
            REQUIRE(ms.count(i) == 1);
        }
    }

    SECTION("Alternating insert and remove")
    {
        ms.insert(10);
        ms.remove(10);
        ms.insert(10);
        ms.remove(10);
        ms.insert(10);

        REQUIRE(ms.count(10) == 1);
        REQUIRE(ms.contains(10));
        REQUIRE(ms.size() == 1);
    }

    SECTION("Mixed operations")
    {
        ms.insert(10);
        ms.insert(20);
        ms.insert(10);
        ms.remove(20);
        ms.insert(30);
        ms.insert(10);
        ms.remove(10);

        REQUIRE(ms.count(10) == 2);
        REQUIRE(ms.count(20) == 0);
        REQUIRE(ms.count(30) == 1);
        REQUIRE(ms.contains(10));
        REQUIRE_FALSE(ms.contains(20));
        REQUIRE(ms.contains(30));
    }
}

TEST_CASE("Multiset incremental hash invariants", "[multiset][hash]")
{
    SECTION("Empty multisets have deterministic hash")
    {
        Multiset ms1, ms2;
        REQUIRE(ms1.hash() == ms2.hash());
    }

    SECTION("Hash is commutative - insertion order independence")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms1.insert(20);
        ms1.insert(30);

        ms2.insert(30);
        ms2.insert(10);
        ms2.insert(20);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Hash is commutative with duplicates")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms1.insert(10);
        ms1.insert(20);

        ms2.insert(20);
        ms2.insert(10);
        ms2.insert(10);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Single insert maintains hash invariant")
    {
        Multiset ms;
        auto h0 = ms.hash();

        ms.insert(42);
        auto h1 = ms.hash();

        REQUIRE(h0 != h1);

        Multiset reference;
        reference.insert(42);
        REQUIRE(ms.hash() == reference.hash());
    }

    SECTION("Bulk insert with count maintains hash invariant")
    {
        Multiset ms1, ms2;

        ms1.insert(10, 5);

        for (int i = 0; i < 5; ++i)
            ms2.insert(10);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Remove maintains hash invariant")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms1.insert(20);
        ms1.insert(30);
        ms1.remove(20);

        ms2.insert(10);
        ms2.insert(30);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Remove and reinsert returns to same hash")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        auto h1 = ms.hash();

        ms.remove(20);
        auto h2 = ms.hash();
        REQUIRE(h1 != h2);

        ms.insert(20);
        auto h3 = ms.hash();
        REQUIRE(h1 == h3);
    }

    SECTION("Constructor from vector produces correct hash")
    {
        Vec<id_t> vec = {10, 20, 30, 10};
        Multiset ms1(vec);

        Multiset ms2;
        ms2.insert(10);
        ms2.insert(10);
        ms2.insert(20);
        ms2.insert(30);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Constructor from iterator range produces correct hash")
    {
        Vec<id_t> vec = {5, 15, 25, 15};
        Multiset ms1(vec.begin(), vec.end());

        Multiset ms2;
        for (auto id : vec)
            ms2.insert(id);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Map operation maintains hash correctness")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);

        bool changed = ms.map([](id_t x) { return x + 100; });
        REQUIRE(changed);

        Multiset reference;
        reference.insert(110);
        reference.insert(120);
        reference.insert(130);

        REQUIRE(ms.hash() == reference.hash());
        REQUIRE(ms == reference);
    }

    SECTION("Map with merging maintains hash correctness")
    {
        Multiset ms;
        ms.insert(1);
        ms.insert(2);
        ms.insert(3);

        bool changed = ms.map([](id_t) { return 100; });
        REQUIRE(changed);

        Multiset reference;
        reference.insert(100, 3);

        REQUIRE(ms.hash() == reference.hash());
        REQUIRE(ms == reference);
    }

    SECTION("Map with no changes preserves hash")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        auto h1 = ms.hash();

        bool changed = ms.map([](id_t x) { return x; });
        REQUIRE_FALSE(changed);

        auto h2 = ms.hash();
        REQUIRE(h1 == h2);
    }

    SECTION("Complex incremental operations maintain hash consistency")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms1.insert(20);
        ms1.insert(30);
        ms1.remove(20);
        ms1.insert(40);
        ms1.insert(10);

        ms2.insert(10, 2);
        ms2.insert(30);
        ms2.insert(40);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }

    SECTION("Msetdiff produces correct hash")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms1.insert(10);
        ms1.insert(20);
        ms1.insert(30);

        ms2.insert(10);
        ms2.insert(20);

        Multiset diff = ms1.msetdiff(ms2);

        Multiset expected;
        expected.insert(10);
        expected.insert(30);

        REQUIRE(diff.hash() == expected.hash());
        REQUIRE(diff == expected);
    }

    SECTION("Different multisets have different hashes (probabilistic)")
    {
        Multiset ms1, ms2;

        ms1.insert(10);
        ms2.insert(20);

        REQUIRE(ms1.hash() != ms2.hash());
    }

    SECTION("Different counts produce different hashes")
    {
        Multiset ms1, ms2;

        ms1.insert(10, 3);
        ms2.insert(10, 5);

        REQUIRE(ms1.hash() != ms2.hash());
    }

    SECTION("Hash remains consistent after temporary removal")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);
        auto original_hash = ms.hash();

        ms.remove(20);
        REQUIRE(ms.hash() != original_hash);

        ms.remove(30);
        REQUIRE(ms.hash() != original_hash);

        ms.insert(30);
        REQUIRE(ms.hash() != original_hash);

        ms.insert(20);
        REQUIRE(ms.hash() == original_hash);
    }

    SECTION("Incremental hash matches batch construction")
    {
        Multiset incremental;
        incremental.insert(5);
        incremental.insert(15);
        incremental.insert(25);
        incremental.insert(15);
        incremental.remove(5);
        incremental.insert(35);

        Vec<id_t> final_elements = {15, 15, 25, 35};
        Multiset batch(final_elements);

        REQUIRE(incremental.hash() == batch.hash());
        REQUIRE(incremental == batch);
    }

    SECTION("Hash stability across multiple operations")
    {
        Multiset ms;

        for (int i = 0; i < 100; ++i)
        {
            ms.insert(i);
        }
        auto h1 = ms.hash();

        for (int i = 0; i < 50; ++i)
        {
            ms.remove(i);
        }
        auto h2 = ms.hash();
        REQUIRE(h1 != h2);

        for (int i = 0; i < 50; ++i)
        {
            ms.insert(i);
        }
        auto h3 = ms.hash();
        REQUIRE(h1 == h3);
    }

    SECTION("Collect and reconstruct preserves hash")
    {
        Multiset ms1;
        ms1.insert(10);
        ms1.insert(20);
        ms1.insert(10);
        ms1.insert(30);

        Vec<id_t> collected = ms1.collect();
        Multiset ms2(collected);

        REQUIRE(ms1.hash() == ms2.hash());
        REQUIRE(ms1 == ms2);
    }
}
