#include <functional>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "set_interface.h"
#include "sorted_set.h"

TEST_CASE("SetInterface basic operations") {
    SortedSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);
    sorted_set.insert(3);
    
    SetInterface set_interface(sorted_set);
    
    SECTION("Size and contains work correctly") {
        REQUIRE(set_interface.size() == 3);
        REQUIRE(set_interface.contains(1) == true);
        REQUIRE(set_interface.contains(2) == true);
        REQUIRE(set_interface.contains(3) == true);
        REQUIRE(set_interface.contains(4) == false);
    }
    
    SECTION("Insert works correctly") {
        REQUIRE(set_interface.insert(4) == true);
        REQUIRE(set_interface.size() == 4);
        REQUIRE(set_interface.contains(4) == true);
        
        // Inserting duplicate should return false
        REQUIRE(set_interface.insert(4) == false);
        REQUIRE(set_interface.size() == 4);
    }
    
    SECTION("for_each works correctly") {
        std::vector<id_t> collected;
        set_interface.for_each([&collected](id_t id) {
            collected.push_back(id);
        });
        
        REQUIRE(collected.size() == 3);
        REQUIRE(collected[0] == 1);
        REQUIRE(collected[1] == 2);
        REQUIRE(collected[2] == 3);
    }
}

TEST_CASE("SetInterface copy and move semantics") {
    SortedSet sorted_set;
    sorted_set.insert(1);
    sorted_set.insert(2);
    
    SetInterface original(sorted_set);
    
    SECTION("Copy constructor works") {
        SetInterface copy(original);
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
        SetInterface copy(empty_set);
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
    
    SetInterface interface1(set1);
    SetInterface interface2(set2);
    SetInterface interface3(set3);
    
    SECTION("Intersection of all three sets") {
        std::vector<std::reference_wrapper<const SetInterface>> sets = {
            std::cref(interface1),
            std::cref(interface2),
            std::cref(interface3)
        };
        
        SetInterface result = intersect(sets);
        
        // Expected intersection: {3, 4}
        REQUIRE(result.size() == 2);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
        REQUIRE(result.contains(1) == false);
        REQUIRE(result.contains(2) == false);
        REQUIRE(result.contains(5) == false);
    }
    
    SECTION("Intersection of two sets") {
        std::vector<std::reference_wrapper<const SetInterface>> sets = {
            std::cref(interface1),
            std::cref(interface2)
        };
        
        SetInterface result = intersect(sets);
        
        // Expected intersection: {2, 3, 4}
        REQUIRE(result.size() == 3);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }
    
    SECTION("Intersection of empty vector") {
        std::vector<std::reference_wrapper<const SetInterface>> empty_sets;
        SetInterface result = intersect(empty_sets);
        REQUIRE(result.size() == 0);
        REQUIRE(result.empty() == true);
    }
    
    SECTION("Intersection of single set") {
        std::vector<std::reference_wrapper<const SetInterface>> sets = {
            std::cref(interface1)
        };
        
        SetInterface result = intersect(sets);
        REQUIRE(result.size() == 4);
        REQUIRE(result.contains(1) == true);
        REQUIRE(result.contains(2) == true);
        REQUIRE(result.contains(3) == true);
        REQUIRE(result.contains(4) == true);
    }
}

TEST_CASE("SetInterface copy_into functionality") {
    SortedSet original_set;
    original_set.insert(10);
    original_set.insert(20);
    original_set.insert(30);
    
    SetInterface set_interface(original_set);
    
    SECTION("Member function copy_into works") {
        SortedSet copied_set = set_interface.copy_into<SortedSet>();
        
        REQUIRE(copied_set.size() == 3);
        REQUIRE(copied_set.contains(10) == true);
        REQUIRE(copied_set.contains(20) == true);
        REQUIRE(copied_set.contains(30) == true);
        
        // Modifying copied set shouldn't affect the interface
        copied_set.insert(40);
        REQUIRE(copied_set.size() == 4);
        REQUIRE(set_interface.size() == 3);
    }
    
    SECTION("Free function copy_into works") {
        SortedSet copied_set = copy_into<SortedSet>(set_interface);
        
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