#include <catch2/catch_test_macros.hpp>

#include "indices/trie_index.h"

// Dummy symbol for testing
constexpr Symbol DUMMY_SYMBOL = 0;

TEST_CASE("TrieNode empty", "[trie_node]")
{
    TrieNode node;

    REQUIRE(node.keys.empty());
    REQUIRE(node.children.empty());
    REQUIRE(node.find_key_index(0) == -1);
    REQUIRE(node.find_key_index(1) == -1);
    REQUIRE(node.find_key_index(9999) == -1);
}

TEST_CASE("TrieNode insert_path single element", "[trie_node]")
{
    TrieNode root;

    SECTION("Insert single element path")
    {
        Vec<id_t> path = {42};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 42);
        REQUIRE(root.children.size() == 1);
        REQUIRE(root.find_key_index(42) == 0);
        REQUIRE(root.find_key_index(43) == -1);
    }

    SECTION("Insert multiple single-element paths")
    {
        root.insert_path({10});
        root.insert_path({30});
        root.insert_path({20});

        // Keys should be sorted
        REQUIRE(root.keys.size() == 3);
        REQUIRE(root.keys[0] == 10);
        REQUIRE(root.keys[1] == 20);
        REQUIRE(root.keys[2] == 30);
        REQUIRE(root.children.size() == 3);

        REQUIRE(root.find_key_index(10) == 0);
        REQUIRE(root.find_key_index(20) == 1);
        REQUIRE(root.find_key_index(30) == 2);
    }

    SECTION("Insert duplicate single-element path")
    {
        root.insert_path({42});
        root.insert_path({42});

        // Should not create duplicates
        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 42);
        REQUIRE(root.children.size() == 1);
    }
}

TEST_CASE("TrieNode insert_path multiple elements", "[trie_node]")
{
    TrieNode root;

    SECTION("Insert two-element path")
    {
        Vec<id_t> path = {10, 20};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 10);
        REQUIRE(root.children.size() == 1);

        TrieNode *child = root.children[0].get();
        REQUIRE(child->keys.size() == 1);
        REQUIRE(child->keys[0] == 20);
        REQUIRE(child->children.size() == 1);
    }

    SECTION("Insert multiple paths with shared prefix")
    {
        root.insert_path({10, 20});
        root.insert_path({10, 30});
        root.insert_path({15, 25});

        // Root should have keys 10, 15
        REQUIRE(root.keys.size() == 2);
        REQUIRE(root.keys[0] == 10);
        REQUIRE(root.keys[1] == 15);
        REQUIRE(root.children.size() == 2);

        // First child (key 10) should have keys 20, 30
        TrieNode *child_10 = root.children[0].get();
        REQUIRE(child_10->keys.size() == 2);
        REQUIRE(child_10->keys[0] == 20);
        REQUIRE(child_10->keys[1] == 30);
        REQUIRE(child_10->children.size() == 2);

        // Second child (key 15) should have key 25
        TrieNode *child_15 = root.children[1].get();
        REQUIRE(child_15->keys.size() == 1);
        REQUIRE(child_15->keys[0] == 25);
        REQUIRE(child_15->children.size() == 1);
    }

    SECTION("Insert three-element path")
    {
        Vec<id_t> path = {1, 2, 3};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 1);

        TrieNode *child1 = root.children[0].get();
        REQUIRE(child1->keys.size() == 1);
        REQUIRE(child1->keys[0] == 2);

        TrieNode *child2 = child1->children[0].get();
        REQUIRE(child2->keys.size() == 1);
        REQUIRE(child2->keys[0] == 3);
        REQUIRE(child2->children.size() == 1);
    }

    SECTION("Insert empty path does nothing")
    {
        Vec<id_t> empty_path;
        root.insert_path(empty_path);

        REQUIRE(root.keys.empty());
        REQUIRE(root.children.empty());
    }
}

TEST_CASE("TrieIndex navigation", "[trie_index]")
{
    auto root = std::make_shared<TrieNode>();
    root->insert_path({10, 20});
    root->insert_path({10, 30});
    root->insert_path({15, 25});

    SECTION("Basic select operation")
    {
        TrieIndex index(DUMMY_SYMBOL, root);

        // REQUIRE_THROWS( index.select(5) );
        index.select(10);
        // REQUIRE_THROWS( index.select(15) );
    }

    SECTION("Project after select")
    {
        TrieIndex index(DUMMY_SYMBOL, root);

        // Initially at root
        AbstractSet keys1 = index.project();
        REQUIRE(keys1.size() == 2);
        REQUIRE(keys1.contains(10));
        REQUIRE(keys1.contains(15));

        // Select key 10 and project
        index.select(10);

        AbstractSet keys2 = index.project();
        REQUIRE(keys2.size() == 2);
        REQUIRE(keys2.contains(20));
        REQUIRE(keys2.contains(30));
    }

    SECTION("Select and backtrack")
    {
        TrieIndex index(DUMMY_SYMBOL, root);

        // Navigate down
        index.select(10);
        AbstractSet child_keys = index.project();
        REQUIRE(child_keys.size() == 2);

        // Backtrack to root
        index.unselect();
        AbstractSet root_keys = index.project();
        REQUIRE(root_keys.size() == 2);
        REQUIRE(root_keys.contains(10));
        REQUIRE(root_keys.contains(15));
    }

    SECTION("Multiple select operations")
    {
        TrieIndex index(DUMMY_SYMBOL, root);

        index.select(10);
        index.select(20);

        // Should be at leaf node
        AbstractSet leaf_keys = index.project();
        REQUIRE(leaf_keys.empty());

        // Backtrack twice to get to root
        index.unselect();
        index.unselect();

        AbstractSet root_keys = index.project();
        REQUIRE(root_keys.size() == 2);
    }
}

TEST_CASE("TrieIndex edge cases", "[trie_index]")
{
    SECTION("Empty node trie")
    {
        auto root = std::make_shared<TrieNode>();
        TrieIndex index(DUMMY_SYMBOL, root);

        AbstractSet keys = index.project();
        REQUIRE(keys.empty());
    }

    SECTION("Trie with only root keys")
    {
        auto root = std::make_shared<TrieNode>();
        root->insert_path({10});
        root->insert_path({20});

        TrieIndex index(DUMMY_SYMBOL, root);
        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 2);
        REQUIRE(keys.contains(10));
        REQUIRE(keys.contains(20));

        // Can select but will be at leaf
        index.select(10);
        AbstractSet leaf_keys = index.project();
        REQUIRE(leaf_keys.empty());
    }
}

TEST_CASE("TrieNode find_key_index edge cases", "[trie_node]")
{
    TrieNode node;

    SECTION("Find in sorted keys")
    {
        node.keys = {1, 3, 5, 7, 9};

        REQUIRE(node.find_key_index(1) == 0);
        REQUIRE(node.find_key_index(3) == 1);
        REQUIRE(node.find_key_index(5) == 2);
        REQUIRE(node.find_key_index(7) == 3);
        REQUIRE(node.find_key_index(9) == 4);

        // Keys not in the list
        REQUIRE(node.find_key_index(0) == -1);
        REQUIRE(node.find_key_index(2) == -1);
        REQUIRE(node.find_key_index(4) == -1);
        REQUIRE(node.find_key_index(6) == -1);
        REQUIRE(node.find_key_index(8) == -1);
        REQUIRE(node.find_key_index(10) == -1);
    }

    SECTION("Find in single-element keys")
    {
        node.keys = {42};

        REQUIRE(node.find_key_index(42) == 0);
        REQUIRE(node.find_key_index(41) == -1);
        REQUIRE(node.find_key_index(43) == -1);
    }
}

TEST_CASE("TrieIndex simultaneous traversal", "[trie_index]")
{
    // Build a trie with multiple paths
    auto root = std::make_shared<TrieNode>();
    root->insert_path({1, 10});
    root->insert_path({1, 20});
    root->insert_path({2, 30});
    root->insert_path({2, 40});

    SECTION("Two independent copies can traverse simultaneously")
    {
        // Create two copies of the index
        TrieIndex index1(DUMMY_SYMBOL, root);
        TrieIndex index2(DUMMY_SYMBOL, root);

        {
            AbstractSet keys1 = index1.project();
            AbstractSet keys2 = index2.project();

            // Both should start at root and see the same keys
            REQUIRE(keys1.size() == 2);
            REQUIRE(keys2.size() == 2);
            REQUIRE(keys1.contains(1));
            REQUIRE(keys2.contains(1));
        }

        // Navigate index1 down path {1, 10}
        index1.select(1);

        {
            AbstractSet keys1 = index1.project();
            AbstractSet keys2 = index2.project();

            REQUIRE(keys1.size() == 2);
            REQUIRE(keys1.contains(10));
            REQUIRE(keys1.contains(20));

            // index2 should still be at root
            REQUIRE(keys2.size() == 2);
            REQUIRE(keys2.contains(1));
            REQUIRE(keys2.contains(2));
        }

        // Navigate index2 down path {2, 30}
        index2.select(2);

        {
            AbstractSet keys1 = index1.project();
            AbstractSet keys2 = index2.project();

            REQUIRE(keys2.size() == 2);
            REQUIRE(keys2.contains(30));
            REQUIRE(keys2.contains(40));

            // index1 should still be at {1, *}
            REQUIRE(keys1.size() == 2);
            REQUIRE(keys1.contains(10));
            REQUIRE(keys1.contains(20));
        }

        // Continue both paths
        index1.select(10);
        index2.select(30);

        // Both should be at leaf nodes now
        REQUIRE(index1.project().empty());
        REQUIRE(index2.project().empty());
    }

    SECTION("Copy constructor creates independent traversal state")
    {
        TrieIndex index1(DUMMY_SYMBOL, root);
        index1.select(1);

        // Create copy while index1 is at position {1, *}
        TrieIndex index2 = index1;

        {
            AbstractSet keys1 = index1.project();
            AbstractSet keys2 = index2.project();

            // Both should be at the same position initially
            REQUIRE(keys1.size() == 2);
            REQUIRE(keys2.size() == 2);
            REQUIRE(keys1.contains(10));
            REQUIRE(keys2.contains(10));
        }

        // Navigate index1 further
        index1.select(10);
        REQUIRE(index1.project().empty());

        {
            // index2 should still be at {1, *}
            AbstractSet keys2 = index2.project();
            REQUIRE(keys2.size() == 2);
            REQUIRE(keys2.contains(10));
            REQUIRE(keys2.contains(20));
        }

        // Navigate index2 differently
        index2.select(20);
        REQUIRE(index2.project().empty());

        // Verify both are at different leaf positions
        // index1 backtrack should go to {1, 10}
        index1.unselect();
        AbstractSet keys1 = index1.project();
        REQUIRE(keys1.size() == 2);
        REQUIRE(keys1.contains(10));

        // index2 backtrack should go to {1, 20}
        index2.unselect();
        AbstractSet keys2 = index2.project();
        REQUIRE(keys2.size() == 2);
        REQUIRE(keys2.contains(20));
    }

    SECTION("Underlying data is shared, not duplicated")
    {
        TrieIndex index1(DUMMY_SYMBOL, root);
        TrieIndex index2 = index1;

        // Verify they share the same root by checking that modifications
        // to the underlying TrieNode are visible to both
        // (We can't directly test this without modifying the trie,
        // but we verify they at least traverse the same structure)

        index1.select(1);
        index2.select(1);

        // Both should see the same children
        AbstractSet keys1 = index1.project();
        AbstractSet keys2 = index2.project();
        REQUIRE(keys1.size() == keys2.size());
        REQUIRE(keys1.contains(10) == keys2.contains(10));
        REQUIRE(keys1.contains(20) == keys2.contains(20));
    }
}
