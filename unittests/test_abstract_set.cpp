#include <functional>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "abstract_set.h"
#include "sorted_set.h"

TEST_CASE("AbstractSet basic operations") {
    SortedSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);
    sorted_set.insert(3);

    AbstractSet abstract_set(sorted_set);

    SECTION("Size and contains work correctly") {
        REQUIRE(abstract_set.size() == 3);
        REQUIRE(abstract_set.contains(1) == true);
        REQUIRE(abstract_set.contains(2) == true);
        REQUIRE(abstract_set.contains(3) == true);
        REQUIRE(abstract_set.contains(4) == false);
    }

    SECTION("Insert works correctly") {
        REQUIRE(abstract_set.insert(4) == true);
        REQUIRE(abstract_set.size() == 4);
        REQUIRE(abstract_set.contains(4) == true);

        // Inserting duplicate should return false
        REQUIRE(abstract_set.insert(4) == false);
        REQUIRE(abstract_set.size() == 4);
    }

    SECTION("for_each works correctly") {
        std::vector<id_t> collected;
        abstract_set.for_each([&collected](id_t id) {
            collected.push_back(id);
        });

        REQUIRE(collected.size() == 3);
        REQUIRE(collected[0] == 1);
        REQUIRE(collected[1] == 2);
        REQUIRE(collected[2] == 3);
    }
}

TEST_CASE("AbstractSet copy and move semantics") {
    SortedSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);

    AbstractSet original(sorted_set);

    SECTION("Copy constructor works") {
        AbstractSet copy(original);
        REQUIRE(copy.size() == 2);
        REQUIRE(copy.contains(1) == true);
        REQUIRE(copy.contains(2) == true);

        // Modifying copy shouldn't affect original
        copy.insert(3);
        REQUIRE(copy.size() == 3);
        REQUIRE(original.size() == 2);
    }

    SECTION("Copy assignment works") {
        SortedSet empty_set;
        AbstractSet copy(empty_set);
        copy = original;

        REQUIRE(copy.size() == 2);
        REQUIRE(copy.contains(1) == true);
        REQUIRE(copy.contains(2) == true);
    }
}

TEST_CASE("Intersection of multiple sets") {
    SortedSet set1, set2, set3;

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

    AbstractSet abstract1(set1);
    AbstractSet abstract2(set2);
    AbstractSet abstract3(set3);

    SECTION("Intersection of all three sets") {
        std::vector<std::reference_wrapper<const AbstractSet>> sets = {
            std::cref(abstract1),
            std::cref(abstract2),
            std::cref(abstract3)
        };

        SortedSet result_set;
        intersect(result_set, sets);
        AbstractSet result(result_set);

        // Expected intersection: {3, 4}
        REQUIRE(result.size() == 2);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
        REQUIRE(result.contains(1) == false);
        REQUIRE(result.contains(2) == false);
        REQUIRE(result.contains(5) == false);
    }

    SECTION("Intersection of two sets") {
        std::vector<std::reference_wrapper<const AbstractSet>> sets = {
            std::cref(abstract1),
            std::cref(abstract2)
        };

        SortedSet result_set;
        intersect(result_set, sets);
        AbstractSet result(result_set);

        // Expected intersection: {2, 3, 4}
        REQUIRE(result.size() == 3);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }

    SECTION("Intersection of empty vector") {
        std::vector<std::reference_wrapper<const AbstractSet>> empty_sets;
        SortedSet result_set;
        intersect(result_set, empty_sets);
        AbstractSet result(result_set);
        REQUIRE(result.size() == 0);
        REQUIRE(result.empty() == true);
    }

    SECTION("Intersection of single set") {
        std::vector<std::reference_wrapper<const AbstractSet>> sets = {
            std::cref(abstract1)
        };

        SortedSet result_set;
        intersect(result_set, sets);
        AbstractSet result(result_set);
        REQUIRE(result.size() == 4);
        REQUIRE(result.contains(1) == true);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }
}

TEST_CASE("AbstractSet copy_into functionality") {
    SortedSet original_set;
    original_set.insert(10);
    original_set.insert(20);
    original_set.insert(30);

    AbstractSet abstract_set(original_set);

    SECTION("Member function copy_into works") {
        SortedSet copied_set = abstract_set.copy_into<SortedSet>();

        REQUIRE(copied_set.size() == 3);
        REQUIRE(copied_set.contains(10) == true);
        REQUIRE(copied_set.contains(20) == true);
        REQUIRE(copied_set.contains(30) == true);

        // Modifying copied set shouldn't affect the interface
        copied_set.insert(40);
        REQUIRE(copied_set.size() == 4);
        REQUIRE(abstract_set.size() == 3);
    }

    SECTION("Free function copy_into works") {
        SortedSet copied_set = copy_into<SortedSet>(abstract_set);

        REQUIRE(copied_set.size() == 3);
        REQUIRE(copied_set.contains(10) == true);
        REQUIRE(copied_set.contains(20) == true);
        REQUIRE(copied_set.contains(30) == true);

        // Verify elements are in sorted order
        auto data = copied_set.get_data();
        REQUIRE(data[0] == 10);
        REQUIRE(data[1] == 20);
        REQUIRE(data[2] == 30);
    }
}
