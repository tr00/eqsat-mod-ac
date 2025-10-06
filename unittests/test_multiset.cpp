#include <catch2/catch_test_macros.hpp>

#include "utils/multiset.h"

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

        REQUIRE(ms.size() == 1); // Only one unique id
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
        REQUIRE(ms.size() == 1); // Pair still exists

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
        REQUIRE(ms.size() == 1);
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
        REQUIRE(ms.size() == 3); // All three ids still in data
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

        REQUIRE(ms.size() == 3);

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
        REQUIRE(ms.size() == 1);

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
