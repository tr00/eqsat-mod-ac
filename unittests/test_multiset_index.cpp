#include <catch2/catch_test_macros.hpp>

#include "indices/multiset_index.h"
#include <memory>

TEST_CASE("MultisetIndex basic operations", "[multiset_index]")
{
    // New structure: HashMap<eclass_id, HashMap<term_id, Multiset>>
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();

    SECTION("Single eclass with single term")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(20);
        ms.insert(30);

        HashMap<id_t, Multiset> terms;
        terms[100] = std::move(ms); // term_id = 100

        (*rel)[1] = std::move(terms); // eclass_id = 1

        MultisetIndex index(rel);

        // Select eclass-id
        index.select(1);
        // Select term-id
        index.select(100);

        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 3);
        REQUIRE(keys.contains(10));
        REQUIRE(keys.contains(20));
        REQUIRE(keys.contains(30));
    }

    SECTION("Multiple eclasses and terms")
    {
        // Eclass 1, term 100
        Multiset ms1;
        ms1.insert(10);
        ms1.insert(20);

        // Eclass 1, term 101
        Multiset ms2;
        ms2.insert(30);
        ms2.insert(40);

        HashMap<id_t, Multiset> terms1;
        terms1[100] = std::move(ms1);
        terms1[101] = std::move(ms2);

        (*rel)[1] = std::move(terms1);

        MultisetIndex index(rel);

        // Select first term
        index.select(1);
        index.select(100);
        AbstractSet keys1 = index.project();
        REQUIRE(keys1.size() == 2);
        REQUIRE(keys1.contains(10));
        REQUIRE(keys1.contains(20));

        // Reset and select second term
        index.reset();
        index.select(1);
        index.select(101);
        AbstractSet keys2 = index.project();
        REQUIRE(keys2.size() == 2);
        REQUIRE(keys2.contains(30));
        REQUIRE(keys2.contains(40));
    }
}

TEST_CASE("MultisetIndex select and unselect", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();
    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);
    ms.insert(10); // Add duplicate

    HashMap<id_t, Multiset> terms;
    terms[100] = std::move(ms);
    (*rel)[1] = std::move(terms);

    MultisetIndex index(rel);

    SECTION("Select eclass and term sets mset")
    {
        index.select(1);
        index.select(100);

        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 3);
        REQUIRE(keys.contains(10));
        REQUIRE(keys.contains(20));
        REQUIRE(keys.contains(30));
    }

    SECTION("Select child removes element from multiset")
    {
        index.select(1);
        index.select(100);

        // Check initial state - 10 should have count 2
        AbstractSet keys1 = index.project();
        REQUIRE(keys1.contains(10));

        // Select 10 - should decrement its count
        index.select(10);

        AbstractSet keys2 = index.project();
        // 10 should still be present (count was 2, now 1)
        REQUIRE(keys2.contains(10));
        REQUIRE(keys2.contains(20));
        REQUIRE(keys2.contains(30));
    }

    SECTION("Select element with count 1 removes it")
    {
        index.select(1);
        index.select(100);
        index.select(20); // 20 has count 1

        AbstractSet keys = index.project();
        REQUIRE(keys.contains(10));
        REQUIRE_FALSE(keys.contains(20)); // Should be removed
        REQUIRE(keys.contains(30));
    }

    SECTION("Unselect restores element")
    {
        index.select(1);
        index.select(100);
        index.select(20);

        // 20 should be removed
        AbstractSet keys1 = index.project();
        REQUIRE_FALSE(keys1.contains(20));

        // Unselect should restore 20
        index.unselect();
        AbstractSet keys2 = index.project();
        REQUIRE(keys2.contains(20));
    }

    SECTION("Multiple select and unselect")
    {
        index.select(1);
        index.select(100);
        index.select(10);
        index.select(20);

        AbstractSet keys1 = index.project();
        REQUIRE(keys1.contains(10)); // Count was 2, now 1
        REQUIRE_FALSE(keys1.contains(20));
        REQUIRE(keys1.contains(30));

        index.unselect(); // Restore 20
        AbstractSet keys2 = index.project();
        REQUIRE(keys2.contains(20));

        index.unselect(); // Restore 10
        AbstractSet keys3 = index.project();
        REQUIRE(keys3.contains(10));
    }

    SECTION("Unselect at term level resets mset")
    {
        index.select(1);
        index.select(100);
        index.unselect();

        // mset should be nullptr again
        // We can verify by selecting again
        index.select(100);
        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 3);
    }
}

TEST_CASE("MultisetIndex reset operation", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();
    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);

    HashMap<id_t, Multiset> terms;
    terms[100] = std::move(ms);
    (*rel)[1] = std::move(terms);

    MultisetIndex index(rel);

    SECTION("Reset without selections does nothing")
    {
        index.reset();
        // Should be able to select normally
        index.select(1);
        index.select(100);
        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 3);
    }

    SECTION("Reset after selections restores all elements")
    {
        index.select(1);
        index.select(100);
        index.select(10);
        index.select(20);

        // Some elements removed
        AbstractSet keys1 = index.project();
        REQUIRE_FALSE(keys1.contains(10));
        REQUIRE_FALSE(keys1.contains(20));
        REQUIRE(keys1.contains(30));

        // Reset should restore everything
        index.reset();
        index.select(1);
        index.select(100);
        AbstractSet keys2 = index.project();
        REQUIRE(keys2.size() == 3);
        REQUIRE(keys2.contains(10));
        REQUIRE(keys2.contains(20));
        REQUIRE(keys2.contains(30));
    }

    SECTION("Reset clears mset pointer")
    {
        index.select(1);
        index.select(100);
        index.select(10);

        index.reset();

        // Should be back at initial state
        index.select(1);
        index.select(100);
        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 3);
    }
}

TEST_CASE("MultisetIndex edge cases", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();

    SECTION("Empty multiset in relation")
    {
        Multiset ms; // Empty
        HashMap<id_t, Multiset> terms;
        terms[100] = std::move(ms);
        (*rel)[1] = std::move(terms);

        MultisetIndex index(rel);
        index.select(1);
        index.select(100);

        AbstractSet keys = index.project();
        REQUIRE(keys.empty());
    }

    SECTION("Multiset with high count elements")
    {
        Multiset ms;
        for (int i = 0; i < 10; ++i)
        {
            ms.insert(42);
        }
        ms.insert(100);

        HashMap<id_t, Multiset> terms;
        terms[200] = std::move(ms);
        (*rel)[1] = std::move(terms);
        MultisetIndex index(rel);

        index.select(1);
        index.select(200);

        // Select 42 multiple times
        for (int i = 0; i < 5; ++i)
        {
            AbstractSet keys = index.project();
            REQUIRE(keys.contains(42));
            index.select(42);
        }

        // 42 should still be present (count was 10, now 5)
        AbstractSet keys = index.project();
        REQUIRE(keys.contains(42));
        REQUIRE(keys.contains(100));

        // Unselect a few times
        for (int i = 0; i < 3; ++i)
        {
            index.unselect();
        }

        AbstractSet keys2 = index.project();
        REQUIRE(keys2.contains(42));
        REQUIRE(keys2.contains(100));
    }

    SECTION("Select same element to zero")
    {
        Multiset ms;
        ms.insert(10);
        ms.insert(10);
        ms.insert(10);

        HashMap<id_t, Multiset> terms;
        terms[100] = std::move(ms);
        (*rel)[1] = std::move(terms);
        MultisetIndex index(rel);

        index.select(1);
        index.select(100);

        // Remove all occurrences
        index.select(10);
        index.select(10);
        index.select(10);

        AbstractSet keys = index.project();
        REQUIRE_FALSE(keys.contains(10));

        // Unselect should restore one at a time
        index.unselect();
        AbstractSet keys2 = index.project();
        REQUIRE(keys2.contains(10));
    }

    SECTION("Complex navigation pattern")
    {
        Multiset ms;
        ms.insert(1);
        ms.insert(2);
        ms.insert(3);
        ms.insert(2); // 2 has count 2

        HashMap<id_t, Multiset> terms;
        terms[500] = std::move(ms);
        (*rel)[100] = std::move(terms);
        MultisetIndex index(rel);

        index.select(100); // eclass-id
        index.select(500); // term-id
        index.select(1);   // Remove 1
        index.select(2);   // Decrement 2 (still present)

        AbstractSet keys1 = index.project();
        REQUIRE_FALSE(keys1.contains(1));
        REQUIRE(keys1.contains(2)); // Still has count 1
        REQUIRE(keys1.contains(3));

        index.unselect(); // Restore 2
        index.select(3);  // Remove 3

        AbstractSet keys2 = index.project();
        REQUIRE_FALSE(keys2.contains(1));
        REQUIRE(keys2.contains(2));
        REQUIRE_FALSE(keys2.contains(3));

        index.reset(); // Restore everything

        index.select(100);
        index.select(500);
        AbstractSet keys3 = index.project();
        REQUIRE(keys3.contains(1));
        REQUIRE(keys3.contains(2));
        REQUIRE(keys3.contains(3));
    }
}

TEST_CASE("MultisetIndex for_each iteration", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();
    Multiset ms;
    ms.insert(10);
    ms.insert(20);
    ms.insert(30);
    ms.insert(10);

    HashMap<id_t, Multiset> terms;
    terms[100] = std::move(ms);
    (*rel)[1] = std::move(terms);
    MultisetIndex index(rel);

    SECTION("Iterate over all elements")
    {
        index.select(1);
        index.select(100);
        AbstractSet keys = index.project();

        Vec<id_t> collected;
        keys.for_each([&](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 3);
        REQUIRE(std::find(collected.begin(), collected.end(), 10) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 20) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 30) != collected.end());
    }

    SECTION("Iterate after removing elements")
    {
        index.select(1);
        index.select(100);
        index.select(20);

        AbstractSet keys = index.project();

        Vec<id_t> collected;
        keys.for_each([&](id_t id) { collected.push_back(id); });

        REQUIRE(collected.size() == 2);
        REQUIRE(std::find(collected.begin(), collected.end(), 10) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 30) != collected.end());
        REQUIRE(std::find(collected.begin(), collected.end(), 20) == collected.end());
    }
}

TEST_CASE("MultisetIndex project at different levels", "[multiset_index]")
{
    auto rel = std::make_shared<HashMap<id_t, HashMap<id_t, Multiset>>>();

    // Setup: eclass 1 with terms 100, 101
    //        eclass 2 with term 200
    Multiset ms1;
    ms1.insert(10);
    ms1.insert(20);

    Multiset ms2;
    ms2.insert(30);

    Multiset ms3;
    ms3.insert(40);
    ms3.insert(50);

    HashMap<id_t, Multiset> terms1;
    terms1[100] = std::move(ms1);
    terms1[101] = std::move(ms2);

    HashMap<id_t, Multiset> terms2;
    terms2[200] = std::move(ms3);

    (*rel)[1] = std::move(terms1);
    (*rel)[2] = std::move(terms2);

    MultisetIndex index(rel);

    SECTION("Project at eclass level shows all eclasses")
    {
        AbstractSet eclasses = index.project();
        REQUIRE(eclasses.size() == 2);
        REQUIRE(eclasses.contains(1));
        REQUIRE(eclasses.contains(2));
    }

    SECTION("Project at term level shows terms for selected eclass")
    {
        index.select(1);
        AbstractSet terms = index.project();
        REQUIRE(terms.size() == 2);
        REQUIRE(terms.contains(100));
        REQUIRE(terms.contains(101));

        index.reset();
        index.select(2);
        AbstractSet terms2 = index.project();
        REQUIRE(terms2.size() == 1);
        REQUIRE(terms2.contains(200));
    }

    SECTION("Project at child level shows children for selected term")
    {
        index.select(1);
        index.select(100);
        AbstractSet children = index.project();
        REQUIRE(children.size() == 2);
        REQUIRE(children.contains(10));
        REQUIRE(children.contains(20));
    }
}
