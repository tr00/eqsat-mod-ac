#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "sets/abstract_set.h"

TEST_CASE("AbstractSet basic operations")
{
    SortedVecSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);
    sorted_set.insert(3);

    AbstractSet abstract_set(sorted_set);

    SECTION("Size and contains work correctly")
    {
        REQUIRE(abstract_set.size() == 3);
        REQUIRE(abstract_set.contains(1) == true);
        REQUIRE(abstract_set.contains(2) == true);
        REQUIRE(abstract_set.contains(3) == true);
        REQUIRE(abstract_set.contains(4) == false);
    }

    SECTION("Creating new AbstractSet with additional element")
    {
        // Create a new SortedVecSet, insert elements, then create AbstractSet
        SortedVecSet new_sorted_set;
        new_sorted_set.insert(1);
        new_sorted_set.insert(2);
        new_sorted_set.insert(3);
        new_sorted_set.insert(4);

        AbstractSet new_abstract_set(std::move(new_sorted_set));

        REQUIRE(new_abstract_set.size() == 4);
        REQUIRE(new_abstract_set.contains(4) == true);
        REQUIRE(new_abstract_set.contains(1) == true);
        REQUIRE(new_abstract_set.contains(2) == true);
        REQUIRE(new_abstract_set.contains(3) == true);
    }

    SECTION("for_each works correctly")
    {
        std::vector<id_t> collected;
        abstract_set.for_each([&collected](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 3);
        REQUIRE(collected[0] == 1);
        REQUIRE(collected[1] == 2);
        REQUIRE(collected[2] == 3);
    }
}

TEST_CASE("AbstractSet move semantics")
{
    SortedVecSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);

    AbstractSet original(std::move(sorted_set));

    SECTION("Move constructor works")
    {
        AbstractSet moved(std::move(original));
        REQUIRE(moved.size() == 2);
        REQUIRE(moved.contains(1) == true);
        REQUIRE(moved.contains(2) == true);
    }
}

TEST_CASE("Intersection of multiple sets")
{
    SortedVecSet set1, set2, set3;

    // Set1: {1, 2, 3, 4}
    set1.insert(1);
    set1.insert(2);
    set1.insert(3);
    set1.insert(4);

    // Set2: {2, 3, 4, 5}
    set2.insert(2);
    set2.insert(3);
    set2.insert(4);
    set2.insert(5);

    // Set3: {3, 4, 5, 6}
    set3.insert(3);
    set3.insert(4);
    set3.insert(5);
    set3.insert(6);

    SECTION("Intersection of all three sets")
    {
        std::vector<AbstractSet> sets;
        sets.emplace_back(AbstractSet(set1));
        sets.emplace_back(AbstractSet(set2));
        sets.emplace_back(AbstractSet(set3));

        SortedVecSet result_set;
        intersect_many(result_set, sets);
        AbstractSet result(result_set);

        // Expected intersection: {3, 4}
        REQUIRE(result.size() == 2);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
        REQUIRE(result.contains(1) == false);
        REQUIRE(result.contains(2) == false);
        REQUIRE(result.contains(5) == false);
    }

    SECTION("Intersection of two sets")
    {
        std::vector<AbstractSet> sets;
        sets.emplace_back(AbstractSet(set1));
        sets.emplace_back(AbstractSet(set2));

        SortedVecSet result_set;
        intersect_many(result_set, sets);
        AbstractSet result(result_set);

        // Expected intersection: {2, 3, 4}
        REQUIRE(result.size() == 3);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }

    SECTION("Intersection of empty vector")
    {
        std::vector<AbstractSet> empty_sets;
        SortedVecSet result_set;
        intersect_many(result_set, empty_sets);
        AbstractSet result(result_set);
        REQUIRE(result.size() == 0);
        REQUIRE(result.empty() == true);
    }

    SECTION("Intersection of single set")
    {
        std::vector<AbstractSet> sets;
        sets.emplace_back(AbstractSet(set1));

        SortedVecSet result_set;
        intersect_many(result_set, sets);
        AbstractSet result(result_set);
        REQUIRE(result.size() == 4);
        REQUIRE(result.contains(1) == true);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }
}
