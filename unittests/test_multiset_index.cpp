#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "enode.h"
#include "indices/multiset_index.h"

TEST_CASE("MultisetIndex basic operations", "[multiset_index]")
{
    // New structure: HashMap<term_id, Multiset>
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 42;

    SECTION("Single term with multiple children")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);

        (*rel)[100] = std::move(ms); // term_id = 100

        MultisetIndex index(test_symbol, rel);

        // Project at term level - should show all term IDs
        AbstractSet terms = index.project();
        REQUIRE(terms.size() == 1);
        REQUIRE(terms.contains(100));

        // Select term and project children
        index.select(100);
        AbstractSet children = index.project();
        REQUIRE(children.size() == 3);
        REQUIRE(children.contains(10));
        REQUIRE(children.contains(20));
        REQUIRE(children.contains(30));
    }

    SECTION("Multiple terms")
    {
        Multiset ms1;
        ms1.insert(10);
        ms1.insert(20);

        Multiset ms2;
        ms2.insert(30);
        ms2.insert(40);

        (*rel)[100] = std::move(ms1);
        (*rel)[101] = std::move(ms2);

        MultisetIndex index(test_symbol, rel);

        // Project all terms
        AbstractSet terms = index.project();
        REQUIRE(terms.size() == 2);
        REQUIRE(terms.contains(100));
        REQUIRE(terms.contains(101));

        // Select first term
        index.select(100);
        AbstractSet children1 = index.project();
        REQUIRE(children1.size() == 2);
        REQUIRE(children1.contains(10));
        REQUIRE(children1.contains(20));

        // Reset and select second term
        index.reset();
        index.select(101);
        AbstractSet children2 = index.project();
        REQUIRE(children2.size() == 2);
        REQUIRE(children2.contains(30));
        REQUIRE(children2.contains(40));
    }

    SECTION("Empty relation")
    {
        MultisetIndex index(test_symbol, rel);
        AbstractSet terms = index.project();
        REQUIRE(terms.empty());
    }
}

TEST_CASE("MultisetIndex select and unselect children", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 1;

    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);
    ms.insert(10); // Add duplicate - 10 now has count 2

    (*rel)[100] = std::move(ms);

    MultisetIndex index(test_symbol, rel);

    SECTION("Select child decrements its count")
    {
        index.select(100); // Select term

        // Check initial state - all children present
        AbstractSet children1 = index.project();
        REQUIRE(children1.contains(10));
        REQUIRE(children1.contains(20));
        REQUIRE(children1.contains(30));

        // Select 10 - should decrement its count from 2 to 1
        index.select(10);

        AbstractSet children2 = index.project();
        // 10 should still be present (count was 2, now 1)
        REQUIRE(children2.contains(10));
        REQUIRE(children2.contains(20));
        REQUIRE(children2.contains(30));
    }

    SECTION("Select element with count 1 removes it")
    {
        index.select(100);
        index.select(20); // 20 has count 1

        AbstractSet children = index.project();
        REQUIRE(children.contains(10));
        REQUIRE_FALSE(children.contains(20)); // Should be removed
        REQUIRE(children.contains(30));
    }

    SECTION("Unselect restores element")
    {
        index.select(100);
        index.select(20);

        // 20 should be removed
        AbstractSet children1 = index.project();
        REQUIRE_FALSE(children1.contains(20));

        // Unselect should restore 20
        index.unselect();
        AbstractSet children2 = index.project();
        REQUIRE(children2.contains(20));
    }

    SECTION("Multiple select and unselect")
    {
        index.select(100);
        index.select(10); // Decrement 10 (2 -> 1)
        index.select(20); // Remove 20

        AbstractSet children1 = index.project();
        REQUIRE(children1.contains(10)); // Still present with count 1
        REQUIRE_FALSE(children1.contains(20));
        REQUIRE(children1.contains(30));

        index.unselect(); // Restore 20
        AbstractSet children2 = index.project();
        REQUIRE(children2.contains(20));

        index.unselect(); // Restore 10's count (1 -> 2)
        AbstractSet children3 = index.project();
        REQUIRE(children3.contains(10));
        REQUIRE(children3.contains(20));
        REQUIRE(children3.contains(30));
    }

    SECTION("Unselect at term level resets mset")
    {
        index.select(100);
        index.unselect(); // Back to term level

        // mset should be nullopt again
        AbstractSet terms = index.project();
        REQUIRE(terms.contains(100));

        // Verify we can select again
        index.select(100);
        AbstractSet children = index.project();
        REQUIRE(children.size() == 3);
    }
}

TEST_CASE("MultisetIndex reset operation", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 5;

    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);
    ms.insert(10); // 10 has count 2

    (*rel)[100] = std::move(ms);

    MultisetIndex index(test_symbol, rel);

    SECTION("Reset without selections does nothing")
    {
        index.reset();
        // Should be able to select normally
        index.select(100);
        AbstractSet children = index.project();
        REQUIRE(children.size() == 3);
    }

    SECTION("Reset after child selections restores all elements")
    {
        index.select(100);
        index.select(10); // Decrement 10
        index.select(10); // Remove 10
        index.select(20); // Remove 20

        // Some elements removed
        AbstractSet children1 = index.project();
        REQUIRE_FALSE(children1.contains(10));
        REQUIRE_FALSE(children1.contains(20));
        REQUIRE(children1.contains(30));

        // Reset should restore everything and return to term level
        index.reset();

        // Should be back at term level
        AbstractSet terms = index.project();
        REQUIRE(terms.contains(100));

        // Select term again and verify all children restored
        index.select(100);
        AbstractSet children2 = index.project();
        REQUIRE(children2.size() == 3);
        REQUIRE(children2.contains(10));
        REQUIRE(children2.contains(20));
        REQUIRE(children2.contains(30));
    }

    SECTION("Reset clears history")
    {
        index.select(100);
        index.select(10);
        index.select(20);

        index.reset();

        // Should be back at initial state (term level)
        AbstractSet terms = index.project();
        REQUIRE(terms.contains(100));

        index.select(100);
        AbstractSet children = index.project();
        REQUIRE(children.size() == 3);
    }
}

TEST_CASE("MultisetIndex edge cases", "[multiset_index]")
{
    Symbol test_symbol = 7;

    SECTION("Empty multiset in relation")
    {
        auto rel = std::make_shared<HashMap<id_t, Multiset>>();
        Multiset ms; // Empty
        (*rel)[100] = std::move(ms);

        MultisetIndex index(test_symbol, rel);
        index.select(100);

        AbstractSet children = index.project();
        REQUIRE(children.empty());
    }

    SECTION("Multiset with high count elements")
    {
        auto rel = std::make_shared<HashMap<id_t, Multiset>>();
        Multiset ms;
        for (int i = 0; i < 10; ++i)
        {
            ms.insert(42);
        }
        ms.insert(100);

        (*rel)[200] = std::move(ms);
        MultisetIndex index(test_symbol, rel);

        index.select(200);

        // Select 42 multiple times
        for (int i = 0; i < 5; ++i)
        {
            AbstractSet children = index.project();
            REQUIRE(children.contains(42));
            index.select(42);
        }

        // 42 should still be present (count was 10, now 5)
        AbstractSet children = index.project();
        REQUIRE(children.contains(42));
        REQUIRE(children.contains(100));

        // Unselect a few times
        for (int i = 0; i < 3; ++i)
        {
            index.unselect();
        }

        AbstractSet children2 = index.project();
        REQUIRE(children2.contains(42));
        REQUIRE(children2.contains(100));
    }

    SECTION("Select same element to zero")
    {
        auto rel = std::make_shared<HashMap<id_t, Multiset>>();
        Multiset ms;
        ms.insert(10);
        ms.insert(10);
        ms.insert(10);

        (*rel)[100] = std::move(ms);
        MultisetIndex index(test_symbol, rel);

        index.select(100);

        // Remove all occurrences
        index.select(10);
        index.select(10);
        index.select(10);

        AbstractSet children = index.project();
        REQUIRE_FALSE(children.contains(10));

        // Unselect should restore one at a time
        index.unselect();
        AbstractSet children2 = index.project();
        REQUIRE(children2.contains(10));

        index.unselect();
        index.unselect();
        AbstractSet children3 = index.project();
        REQUIRE(children3.contains(10));
    }

    SECTION("Complex navigation pattern")
    {
        auto rel = std::make_shared<HashMap<id_t, Multiset>>();
        Multiset ms;
        ms.insert(1);
        ms.insert(2);
        ms.insert(3);
        ms.insert(2); // 2 has count 2

        (*rel)[500] = std::move(ms);
        MultisetIndex index(test_symbol, rel);

        index.select(500); // term-id
        index.select(1);   // Remove 1
        index.select(2);   // Decrement 2 (still present)

        AbstractSet children1 = index.project();
        REQUIRE_FALSE(children1.contains(1));
        REQUIRE(children1.contains(2)); // Still has count 1
        REQUIRE(children1.contains(3));

        index.unselect(); // Restore 2
        index.select(3);  // Remove 3

        AbstractSet children2 = index.project();
        REQUIRE_FALSE(children2.contains(1));
        REQUIRE(children2.contains(2));
        REQUIRE_FALSE(children2.contains(3));

        index.reset(); // Restore everything and back to term level

        index.select(500);
        AbstractSet children3 = index.project();
        REQUIRE(children3.contains(1));
        REQUIRE(children3.contains(2));
        REQUIRE(children3.contains(3));
    }

    SECTION("Single element with high multiplicity")
    {
        auto rel = std::make_shared<HashMap<id_t, Multiset>>();
        Multiset ms;
        for (int i = 0; i < 100; ++i)
        {
            ms.insert(99);
        }

        (*rel)[1] = std::move(ms);
        MultisetIndex index(test_symbol, rel);

        index.select(1);

        // Verify initial state
        AbstractSet children = index.project();
        REQUIRE(children.size() == 1);
        REQUIRE(children.contains(99));

        // Remove many times
        for (int i = 0; i < 50; ++i)
        {
            index.select(99);
        }

        // Should still be present
        AbstractSet children2 = index.project();
        REQUIRE(children2.contains(99));

        // Add them back
        for (int i = 0; i < 50; ++i)
        {
            index.unselect();
        }

        AbstractSet children3 = index.project();
        REQUIRE(children3.contains(99));
    }
}

TEST_CASE("MultisetIndex for_each iteration", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 3;

    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);
    ms.insert(10); // 10 has count 2

    (*rel)[100] = std::move(ms);
    MultisetIndex index(test_symbol, rel);

    SECTION("Iterate over all children")
    {
        index.select(100);
        AbstractSet children = index.project();

        Vec<id_t> collected;
        children.for_each([&](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 3);
        REQUIRE(std::find(collected.begin(), collected.end(), 10) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 20) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 30) != collected.end());
    }

    SECTION("Iterate after removing elements")
    {
        index.select(100);
        index.select(20);

        AbstractSet children = index.project();

        Vec<id_t> collected;
        children.for_each([&](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 2);
        REQUIRE(std::find(collected.begin(), collected.end(), 10) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 30) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 20) == collected.end());
    }

    SECTION("Iterate at term level")
    {
        AbstractSet terms = index.project();

        Vec<id_t> collected;
        terms.for_each([&](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 1);
        REQUIRE(collected[0] == 100);
    }
}

TEST_CASE("MultisetIndex make_enode", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 99;

    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);

    (*rel)[100] = std::move(ms);

    MultisetIndex index(test_symbol, rel);

    SECTION("ENode from empty history")
    {
        ENode enode = index.make_enode();
        REQUIRE(enode.op == test_symbol);
        REQUIRE(enode.children.empty());
    }

    SECTION("ENode after selecting children")
    {
        index.select(100);
        index.select(10);
        index.select(20);

        ENode enode = index.make_enode();
        REQUIRE(enode.op == test_symbol);
        REQUIRE(enode.children.size() == 2);
        REQUIRE(enode.children[0] == 10);
        REQUIRE(enode.children[1] == 20);
    }

    SECTION("ENode history reflects selection order")
    {
        index.select(100);
        index.select(30);
        index.select(10);
        index.select(20);

        ENode enode = index.make_enode();
        REQUIRE(enode.children.size() == 3);
        REQUIRE(enode.children[0] == 30);
        REQUIRE(enode.children[1] == 10);
        REQUIRE(enode.children[2] == 20);
    }

    SECTION("ENode after unselect")
    {
        index.select(100);
        index.select(10);
        index.select(20);
        index.unselect(); // Remove 20 from history

        ENode enode = index.make_enode();
        REQUIRE(enode.children.size() == 1);
        REQUIRE(enode.children[0] == 10);
    }

    SECTION("ENode with duplicates in history")
    {
        Multiset ms2;
        ms2.insert(5);
        ms2.insert(5);
        ms2.insert(5);

        (*rel)[200] = std::move(ms2);
        MultisetIndex index2(test_symbol, rel);

        index2.select(200);
        index2.select(5);
        index2.select(5);

        ENode enode = index2.make_enode();
        REQUIRE(enode.children.size() == 2);
        REQUIRE(enode.children[0] == 5);
        REQUIRE(enode.children[1] == 5);
    }
}

TEST_CASE("MultisetIndex multiple terms with different sizes", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 50;

    // Term 1: small multiset
    Multiset ms1;
    ms1.insert(1);
    (*rel)[10] = std::move(ms1);

    // Term 2: medium multiset
    Multiset ms2;
    ms2.insert(2);
    ms2.insert(3);
    ms2.insert(4);
    (*rel)[20] = std::move(ms2);

    // Term 3: large multiset with duplicates
    Multiset ms3;
    for (id_t i = 5; i < 15; ++i)
    {
        ms3.insert(i);
        ms3.insert(i); // Each has count 2
    }
    (*rel)[30] = std::move(ms3);

    MultisetIndex index(test_symbol, rel);

    SECTION("Project shows all terms")
    {
        AbstractSet terms = index.project();
        REQUIRE(terms.size() == 3);
        REQUIRE(terms.contains(10));
        REQUIRE(terms.contains(20));
        REQUIRE(terms.contains(30));
    }

    SECTION("Navigate through different term sizes")
    {
        // Small term
        index.select(10);
        AbstractSet children1 = index.project();
        REQUIRE(children1.size() == 1);
        REQUIRE(children1.contains(1));

        // Medium term
        index.reset();
        index.select(20);
        AbstractSet children2 = index.project();
        REQUIRE(children2.size() == 3);

        // Large term
        index.reset();
        index.select(30);
        AbstractSet children3 = index.project();
        REQUIRE(children3.size() == 10);
        for (id_t i = 5; i < 15; ++i)
        {
            REQUIRE(children3.contains(i));
        }
    }

    SECTION("Select and unselect across different terms")
    {
        // Work with large term
        index.select(30);
        index.select(5);
        index.select(6);
        index.select(7);

        ENode enode1 = index.make_enode();
        REQUIRE(enode1.children.size() == 3);

        index.reset();

        // Work with medium term
        index.select(20);
        index.select(2);

        ENode enode2 = index.make_enode();
        REQUIRE(enode2.children.size() == 1);
        REQUIRE(enode2.children[0] == 2);
    }
}

TEST_CASE("MultisetIndex stress test with many operations", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, Multiset>>();
    Symbol test_symbol = 77;

    Multiset ms;
    for (id_t i = 0; i < 50; ++i)
    {
        ms.insert(i);
        if (i % 3 == 0)
            ms.insert(i); // Some have count 2
        if (i % 5 == 0)
            ms.insert(i); // Some have count 3
    }

    (*rel)[1000] = std::move(ms);
    MultisetIndex index(test_symbol, rel);

    SECTION("Many selections and unselections")
    {
        index.select(1000);

        // Select many elements
        for (id_t i = 0; i < 20; ++i)
        {
            index.select(i);
        }

        AbstractSet children1 = index.project();
        // Elements 0-19 should have reduced counts or be removed
        for (id_t i = 20; i < 50; ++i)
        {
            REQUIRE(children1.contains(i));
        }

        // Unselect half
        for (int i = 0; i < 10; ++i)
        {
            index.unselect();
        }

        // Reset and verify restoration
        index.reset();
        index.select(1000);
        AbstractSet children2 = index.project();
        REQUIRE(children2.size() == 50);
    }

    SECTION("Build complex ENode")
    {
        index.select(1000);

        Vec<id_t> expected_children;
        for (id_t i = 0; i < 15; ++i)
        {
            index.select(i);
            expected_children.push_back(i);
        }

        ENode enode = index.make_enode();
        REQUIRE(enode.op == test_symbol);
        REQUIRE(enode.children == expected_children);
    }
}
