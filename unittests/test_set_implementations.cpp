#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "sets/abstract_set.h"
#include "utils/multiset.h"

// ============================================================================
// Test Suite Template - tests behavior independent of implementation
// ============================================================================

template <typename SetFactory>
void test_empty_set(SetFactory factory)
{
    auto set = factory({});
    REQUIRE(set.size() == 0);
    REQUIRE(set.empty() == true);
    REQUIRE(set.contains(0) == false);
    REQUIRE(set.contains(1) == false);
    REQUIRE(set.contains(999) == false);

    // for_each on empty set should not call function
    int call_count = 0;
    set.for_each([&call_count](id_t) { call_count++; });
    REQUIRE(call_count == 0);
}

template <typename SetFactory>
void test_single_element(SetFactory factory)
{
    auto set = factory({42});
    REQUIRE(set.size() == 1);
    REQUIRE(set.empty() == false);
    REQUIRE(set.contains(42) == true);
    REQUIRE(set.contains(0) == false);
    REQUIRE(set.contains(41) == false);
    REQUIRE(set.contains(43) == false);

    Vec<id_t> collected;
    set.for_each([&collected](id_t id) { collected.push_back(id); });
    REQUIRE(collected.size() == 1);
    REQUIRE(collected[0] == 42);
}

template <typename SetFactory>
void test_multiple_elements(SetFactory factory)
{
    auto set = factory({5, 2, 8, 1, 9});
    REQUIRE(set.size() == 5);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(2) == true);
    REQUIRE(set.contains(5) == true);
    REQUIRE(set.contains(8) == true);
    REQUIRE(set.contains(9) == true);
    REQUIRE(set.contains(0) == false);
    REQUIRE(set.contains(3) == false);
    REQUIRE(set.contains(10) == false);
}

template <typename SetFactory>
void test_sorted_traversal(SetFactory factory)
{
    // Elements inserted in random order
    auto set = factory({7, 3, 9, 1, 5, 2, 8, 4, 6});

    Vec<id_t> collected;
    set.for_each([&collected](id_t id) { collected.push_back(id); });

    REQUIRE(collected.size() == 9);
    // Verify sorted order
    for (size_t i = 0; i < collected.size(); ++i)
    {
        REQUIRE(collected[i] == static_cast<id_t>(i + 1));
    }

    // Verify it's actually sorted
    REQUIRE(std::is_sorted(collected.begin(), collected.end()) == true);
}

template <typename SetFactory>
void test_boundary_values(SetFactory factory)
{
    auto set = factory({0, 1, std::numeric_limits<id_t>::max()});
    REQUIRE(set.size() == 3);
    REQUIRE(set.contains(0) == true);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(std::numeric_limits<id_t>::max()) == true);
    REQUIRE(set.contains(2) == false);
    REQUIRE(set.contains(std::numeric_limits<id_t>::max() - 1) == false);
}

template <typename SetFactory>
void test_large_set(SetFactory factory)
{
    Vec<id_t> elements;
    for (id_t i = 0; i < 1000; ++i)
    {
        elements.push_back(i * 2); // Even numbers
    }

    auto set = factory(elements);
    REQUIRE(set.size() == 1000);

    // Check all even numbers present
    for (id_t i = 0; i < 1000; ++i)
    {
        REQUIRE(set.contains(i * 2) == true);
        REQUIRE(set.contains(i * 2 + 1) == false); // Odd numbers absent
    }

    // Verify for_each traversal
    size_t count = 0;
    id_t last = 0;
    bool first = true;
    set.for_each([&](id_t id) {
        if (!first)
        {
            REQUIRE(id > last); // Verify sorted order
        }
        first = false;
        last = id;
        count++;
    });
    REQUIRE(count == 1000);
}

template <typename SetFactory>
void test_consecutive_elements(SetFactory factory)
{
    auto set = factory({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    REQUIRE(set.size() == 10);

    for (id_t i = 1; i <= 10; ++i)
    {
        REQUIRE(set.contains(i) == true);
    }
    REQUIRE(set.contains(0) == false);
    REQUIRE(set.contains(11) == false);
}

template <typename SetFactory>
void test_sparse_elements(SetFactory factory)
{
    auto set = factory({1, 100, 1000, 10000, 100000});
    REQUIRE(set.size() == 5);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(100) == true);
    REQUIRE(set.contains(1000) == true);
    REQUIRE(set.contains(10000) == true);
    REQUIRE(set.contains(100000) == true);

    // Check gaps
    REQUIRE(set.contains(2) == false);
    REQUIRE(set.contains(50) == false);
    REQUIRE(set.contains(999) == false);
    REQUIRE(set.contains(9999) == false);
}

template <typename SetFactory>
void test_for_each_consistency(SetFactory factory)
{
    auto set = factory({10, 20, 30, 40, 50});

    // Call for_each multiple times - should be consistent
    Vec<id_t> first_pass, second_pass;
    set.for_each([&first_pass](id_t id) { first_pass.push_back(id); });
    set.for_each([&second_pass](id_t id) { second_pass.push_back(id); });

    REQUIRE(first_pass == second_pass);
}

// ============================================================================
// SortedVecSet Specific Tests
// ============================================================================

TEST_CASE("SortedVecSet - empty set", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>&) { return AbstractSet(SortedVecSet{}); };
    test_empty_set(factory);
}

TEST_CASE("SortedVecSet - single element", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_single_element(factory);
}

TEST_CASE("SortedVecSet - multiple elements", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_multiple_elements(factory);
}

TEST_CASE("SortedVecSet - sorted traversal", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_sorted_traversal(factory);
}

TEST_CASE("SortedVecSet - boundary values", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_boundary_values(factory);
}

TEST_CASE("SortedVecSet - large set", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_large_set(factory);
}

TEST_CASE("SortedVecSet - consecutive elements", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_consecutive_elements(factory);
}

TEST_CASE("SortedVecSet - sparse elements", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_sparse_elements(factory);
}

TEST_CASE("SortedVecSet - for_each consistency", "[set][sortedvec]")
{
    auto factory = [](const Vec<id_t>& elements) {
        SortedVecSet set;
        for (auto id : elements)
            set.insert(id);
        return AbstractSet(std::move(set));
    };
    test_for_each_consistency(factory);
}

TEST_CASE("SortedVecSet - duplicate insertion", "[set][sortedvec]")
{
    SortedVecSet set;
    REQUIRE(set.insert(5) == true);  // First insertion succeeds
    REQUIRE(set.insert(5) == false); // Duplicate insertion fails
    REQUIRE(set.size() == 1);
    REQUIRE(set.contains(5) == true);

    // Try inserting multiple times
    set.insert(5);
    set.insert(5);
    set.insert(5);
    REQUIRE(set.size() == 1);
}

TEST_CASE("SortedVecSet - clear operation", "[set][sortedvec]")
{
    SortedVecSet set;
    set.insert(1);
    set.insert(2);
    set.insert(3);
    REQUIRE(set.size() == 3);

    set.clear();
    REQUIRE(set.size() == 0);
    REQUIRE(set.empty() == true);
    REQUIRE(set.contains(1) == false);
    REQUIRE(set.contains(2) == false);
    REQUIRE(set.contains(3) == false);
}

TEST_CASE("SortedVecSet - iterators", "[set][sortedvec]")
{
    SortedVecSet set;
    set.insert(3);
    set.insert(1);
    set.insert(2);

    Vec<id_t> collected;
    for (auto it = set.begin(); it != set.end(); ++it)
    {
        collected.push_back(*it);
    }

    REQUIRE(collected.size() == 3);
    REQUIRE(collected[0] == 1);
    REQUIRE(collected[1] == 2);
    REQUIRE(collected[2] == 3);
}

// ============================================================================
// SortedIterSet Specific Tests
// Note: SortedIterSet holds iterators, so we need to keep data alive
// ============================================================================

TEST_CASE("SortedIterSet - empty set", "[set][sortediter]")
{
    Vec<id_t> data;
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 0);
    REQUIRE(set.empty() == true);
    REQUIRE(set.contains(0) == false);
}

TEST_CASE("SortedIterSet - single element", "[set][sortediter]")
{
    Vec<id_t> data = {42};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 1);
    REQUIRE(set.contains(42) == true);
    REQUIRE(set.contains(0) == false);
}

TEST_CASE("SortedIterSet - multiple elements", "[set][sortediter]")
{
    Vec<id_t> data = {1, 2, 5, 8, 9};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 5);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(2) == true);
    REQUIRE(set.contains(5) == true);
    REQUIRE(set.contains(8) == true);
    REQUIRE(set.contains(9) == true);
    REQUIRE(set.contains(3) == false);
}

TEST_CASE("SortedIterSet - sorted traversal", "[set][sortediter]")
{
    Vec<id_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    Vec<id_t> collected;
    set.for_each([&collected](id_t id) { collected.push_back(id); });

    REQUIRE(collected.size() == 9);
    REQUIRE(std::is_sorted(collected.begin(), collected.end()) == true);
    for (size_t i = 0; i < collected.size(); ++i)
    {
        REQUIRE(collected[i] == static_cast<id_t>(i + 1));
    }
}

TEST_CASE("SortedIterSet - boundary values", "[set][sortediter]")
{
    Vec<id_t> data = {0, 1, std::numeric_limits<id_t>::max()};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 3);
    REQUIRE(set.contains(0) == true);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(std::numeric_limits<id_t>::max()) == true);
}

TEST_CASE("SortedIterSet - large set", "[set][sortediter]")
{
    Vec<id_t> data;
    for (id_t i = 0; i < 1000; ++i)
    {
        data.push_back(i * 2);
    }

    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));
    REQUIRE(set.size() == 1000);

    for (id_t i = 0; i < 1000; ++i)
    {
        REQUIRE(set.contains(i * 2) == true);
        REQUIRE(set.contains(i * 2 + 1) == false);
    }
}

TEST_CASE("SortedIterSet - consecutive elements", "[set][sortediter]")
{
    Vec<id_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 10);
    for (id_t i = 1; i <= 10; ++i)
    {
        REQUIRE(set.contains(i) == true);
    }
}

TEST_CASE("SortedIterSet - sparse elements", "[set][sortediter]")
{
    Vec<id_t> data = {1, 100, 1000, 10000, 100000};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    REQUIRE(set.size() == 5);
    REQUIRE(set.contains(1) == true);
    REQUIRE(set.contains(100) == true);
    REQUIRE(set.contains(1000) == true);
    REQUIRE(set.contains(10000) == true);
    REQUIRE(set.contains(100000) == true);
    REQUIRE(set.contains(50) == false);
}

TEST_CASE("SortedIterSet - for_each consistency", "[set][sortediter]")
{
    Vec<id_t> data = {10, 20, 30, 40, 50};
    SortedIterSet iter_set(data);
    AbstractSet set(std::move(iter_set));

    Vec<id_t> first_pass, second_pass;
    set.for_each([&first_pass](id_t id) { first_pass.push_back(id); });
    set.for_each([&second_pass](id_t id) { second_pass.push_back(id); });

    REQUIRE(first_pass == second_pass);
}

// ============================================================================
// MultisetSupport Specific Tests
// ============================================================================

TEST_CASE("MultisetSupport - empty set", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>&) {
        mset.clear();
        return AbstractSet(MultisetSupport(mset));
    };
    test_empty_set(factory);
}

TEST_CASE("MultisetSupport - single element", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_single_element(factory);
}

TEST_CASE("MultisetSupport - multiple elements", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_multiple_elements(factory);
}

TEST_CASE("MultisetSupport - sorted traversal", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_sorted_traversal(factory);
}

TEST_CASE("MultisetSupport - boundary values", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_boundary_values(factory);
}

TEST_CASE("MultisetSupport - large set", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_large_set(factory);
}

TEST_CASE("MultisetSupport - consecutive elements", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_consecutive_elements(factory);
}

TEST_CASE("MultisetSupport - sparse elements", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_sparse_elements(factory);
}

TEST_CASE("MultisetSupport - for_each consistency", "[set][multiset]")
{
    Multiset mset;
    auto factory = [&mset](const Vec<id_t>& elements) {
        mset.clear();
        for (auto id : elements)
            mset.insert(id);
        return AbstractSet(MultisetSupport(mset));
    };
    test_for_each_consistency(factory);
}

TEST_CASE("MultisetSupport - handles multiplicities correctly", "[set][multiset]")
{
    Multiset mset;
    mset.insert(5);
    mset.insert(5);
    mset.insert(5);
    mset.insert(10);

    MultisetSupport msupport(mset);
    AbstractSet set(std::move(msupport));

    // Should only report each element once, despite multiplicity
    REQUIRE(set.contains(5) == true);
    REQUIRE(set.contains(10) == true);

    Vec<id_t> collected;
    set.for_each([&collected](id_t id) { collected.push_back(id); });

    // Each element should appear only once in traversal
    REQUIRE(collected.size() == 2);
    REQUIRE(std::count(collected.begin(), collected.end(), 5) == 1);
    REQUIRE(std::count(collected.begin(), collected.end(), 10) == 1);
}

TEST_CASE("MultisetSupport - removed elements not visible", "[set][multiset]")
{
    Multiset mset;
    mset.insert(5);
    mset.insert(5);
    mset.remove(5);
    mset.remove(5);

    MultisetSupport msupport(mset);
    AbstractSet set(std::move(msupport));

    // Element removed completely - should not be visible
    REQUIRE(set.contains(5) == false);

    Vec<id_t> collected;
    set.for_each([&collected](id_t id) { collected.push_back(id); });
    REQUIRE(collected.size() == 0);
}

// ============================================================================
// AbstractSet Move Semantics Tests
// ============================================================================

TEST_CASE("AbstractSet - move constructor SortedVecSet", "[set][abstract][move]")
{
    SortedVecSet vec_set;
    vec_set.insert(1);
    vec_set.insert(2);
    vec_set.insert(3);

    AbstractSet original(std::move(vec_set));
    AbstractSet moved(std::move(original));

    REQUIRE(moved.size() == 3);
    REQUIRE(moved.contains(1) == true);
    REQUIRE(moved.contains(2) == true);
    REQUIRE(moved.contains(3) == true);
}

// ============================================================================
// intersect_many Tests
// ============================================================================

TEST_CASE("intersect_many - empty input", "[set][intersect]")
{
    Vec<AbstractSet> sets;
    SortedVecSet result;
    intersect_many(result, sets);

    REQUIRE(result.size() == 0);
    REQUIRE(result.empty() == true);
}

TEST_CASE("intersect_many - single set", "[set][intersect]")
{
    SortedVecSet s1;
    s1.insert(1);
    s1.insert(2);
    s1.insert(3);

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));

    SortedVecSet result;
    intersect_many(result, sets);

    REQUIRE(result.size() == 3);
    REQUIRE(result.contains(1) == true);
    REQUIRE(result.contains(2) == true);
    REQUIRE(result.contains(3) == true);
}

TEST_CASE("intersect_many - no common elements", "[set][intersect]")
{
    SortedVecSet s1, s2, s3;
    s1.insert(1);
    s1.insert(2);

    s2.insert(3);
    s2.insert(4);

    s3.insert(5);
    s3.insert(6);

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(std::move(s2)));
    sets.emplace_back(AbstractSet(std::move(s3)));

    SortedVecSet result;
    intersect_many(result, sets);

    REQUIRE(result.size() == 0);
    REQUIRE(result.empty() == true);
}

TEST_CASE("intersect_many - all elements common", "[set][intersect]")
{
    SortedVecSet s1, s2, s3;
    for (int i = 1; i <= 5; ++i)
    {
        s1.insert(i);
        s2.insert(i);
        s3.insert(i);
    }

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(std::move(s2)));
    sets.emplace_back(AbstractSet(std::move(s3)));

    SortedVecSet result;
    intersect_many(result, sets);

    REQUIRE(result.size() == 5);
    for (int i = 1; i <= 5; ++i)
    {
        REQUIRE(result.contains(i) == true);
    }
}

TEST_CASE("intersect_many - mixed set types", "[set][intersect]")
{
    // SortedVecSet
    SortedVecSet s1;
    s1.insert(1);
    s1.insert(2);
    s1.insert(3);
    s1.insert(4);

    // SortedIterSet - keep data alive
    Vec<id_t> data = {2, 3, 4, 5};

    // MultisetSupport - keep mset alive
    Multiset mset;
    mset.insert(3);
    mset.insert(4);
    mset.insert(5);
    mset.insert(6);

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(SortedIterSet(data)));
    sets.emplace_back(AbstractSet(MultisetSupport(mset)));

    SortedVecSet result;
    intersect_many(result, sets);

    // Common elements: {3, 4}
    REQUIRE(result.size() == 2);
    REQUIRE(result.contains(3) == true);
    REQUIRE(result.contains(4) == true);
    REQUIRE(result.contains(1) == false);
    REQUIRE(result.contains(2) == false);
    REQUIRE(result.contains(5) == false);
}

TEST_CASE("intersect_many - large sets", "[set][intersect]")
{
    SortedVecSet s1, s2, s3;

    // s1: 0, 2, 4, 6, ... (even numbers: 0 to 5998)
    for (id_t i = 0; i < 3000; ++i)
    {
        s1.insert(i * 2);
    }

    // s2: 0, 3, 6, 9, ... (multiples of 3: 0 to 5997)
    for (id_t i = 0; i < 2000; ++i)
    {
        s2.insert(i * 3);
    }

    // s3: 0, 6, 12, 18, ... (multiples of 6: 0 to 5994)
    for (id_t i = 0; i < 1000; ++i)
    {
        s3.insert(i * 6);
    }

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(std::move(s2)));
    sets.emplace_back(AbstractSet(std::move(s3)));

    SortedVecSet result;
    intersect_many(result, sets);

    // Common elements: multiples of 6 (smallest set is all multiples of 6)
    REQUIRE(result.size() == 1000);
    for (id_t i = 0; i < 1000; ++i)
    {
        REQUIRE(result.contains(i * 6) == true);
    }
}

TEST_CASE("intersect_many - result is sorted", "[set][intersect]")
{
    SortedVecSet s1, s2;
    s1.insert(10);
    s1.insert(5);
    s1.insert(15);
    s1.insert(3);

    s2.insert(15);
    s2.insert(3);
    s2.insert(20);
    s2.insert(5);

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(std::move(s2)));

    SortedVecSet result;
    intersect_many(result, sets);

    Vec<id_t> collected;
    for (auto it = result.begin(); it != result.end(); ++it)
    {
        collected.push_back(*it);
    }

    REQUIRE(std::is_sorted(collected.begin(), collected.end()) == true);
    REQUIRE(collected.size() == 3);
    REQUIRE(collected[0] == 3);
    REQUIRE(collected[1] == 5);
    REQUIRE(collected[2] == 15);
}

TEST_CASE("intersect_many - repeated calls with clear", "[set][intersect]")
{
    SortedVecSet s1, s2;
    s1.insert(1);
    s1.insert(2);
    s2.insert(2);
    s2.insert(3);

    Vec<AbstractSet> sets;
    sets.emplace_back(AbstractSet(std::move(s1)));
    sets.emplace_back(AbstractSet(std::move(s2)));

    SortedVecSet result;

    // First call
    intersect_many(result, sets);
    REQUIRE(result.size() == 1);
    REQUIRE(result.contains(2) == true);

    // Prepare for second call
    SortedVecSet s3, s4;
    s3.insert(5);
    s3.insert(6);
    s4.insert(6);
    s4.insert(7);

    Vec<AbstractSet> sets2;
    sets2.emplace_back(AbstractSet(std::move(s3)));
    sets2.emplace_back(AbstractSet(std::move(s4)));

    // Second call - should clear previous result
    intersect_many(result, sets2);
    REQUIRE(result.size() == 1);
    REQUIRE(result.contains(6) == true);
    REQUIRE(result.contains(2) == false); // Old result cleared
}
