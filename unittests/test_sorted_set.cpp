#include <algorithm>
#include <catch2/catch_test_macros.hpp>

#include "sets/sorted_set.h"

TEST_CASE("SortedSet insertion", "[sorted_set]")
{
    SortedVecSet set;

    SECTION("Insert single element")
    {
        REQUIRE(set.insert(42));
        REQUIRE(set.size() == 1);
        REQUIRE(set.contains(42));
    }

    SECTION("Insert multiple elements")
    {
        REQUIRE(set.insert(30));
        REQUIRE(set.insert(10));
        REQUIRE(set.insert(20));

        REQUIRE(set.size() == 3);
        REQUIRE(set.contains(10));
        REQUIRE(set.contains(20));
        REQUIRE(set.contains(30));
    }

    SECTION("Insert duplicate elements")
    {
        REQUIRE(set.insert(15));
        REQUIRE_FALSE(set.insert(15));

        REQUIRE(set.size() == 1);
        REQUIRE(set.contains(15));
    }

    SECTION("Elements are stored in sorted order")
    {
        set.insert(50);
        set.insert(10);
        set.insert(30);
        set.insert(20);

        REQUIRE(std::is_sorted(set.begin(), set.end()));
    }
}

TEST_CASE("SortedSet membership", "[sorted_set]")
{
    SortedVecSet set;

    SECTION("Empty set contains nothing")
    {
        REQUIRE_FALSE(set.contains(0));
        REQUIRE_FALSE(set.contains(100));
    }

    SECTION("Contains inserted elements")
    {
        set.insert(5);
        set.insert(15);
        set.insert(25);

        REQUIRE(set.contains(5));
        REQUIRE(set.contains(15));
        REQUIRE(set.contains(25));
    }

    SECTION("Does not contain non-inserted elements")
    {
        set.insert(10);
        set.insert(20);

        REQUIRE_FALSE(set.contains(5));
        REQUIRE_FALSE(set.contains(15));
        REQUIRE_FALSE(set.contains(25));
    }
}

TEST_CASE("SortedSet utilities", "[sorted_set]")
{
    SortedVecSet set;

    SECTION("Empty set properties")
    {
        REQUIRE(set.empty());
        REQUIRE(set.size() == 0);
    }

    SECTION("Non-empty set properties")
    {
        set.insert(1);
        set.insert(2);

        REQUIRE_FALSE(set.empty());
        REQUIRE(set.size() == 2);
    }

    SECTION("Clear functionality")
    {
        set.insert(1);
        set.insert(2);
        set.clear();

        REQUIRE(set.empty());
        REQUIRE(set.size() == 0);
        REQUIRE_FALSE(set.contains(1));
        REQUIRE_FALSE(set.contains(2));
    }

    SECTION("Iterator functionality")
    {
        set.insert(30);
        set.insert(10);
        set.insert(20);

        std::vector<id_t> values;
        for (auto id : set)
        {
            values.push_back(id);
        }

        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }
}
