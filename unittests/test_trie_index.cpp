#include <catch2/catch_test_macros.hpp>

#include "indices/trie_index.h"

TEST_CASE("TrieNode basic operations", "[trie_node]") {
    TrieNode node;

    SECTION("Empty node has no keys or children") {
        REQUIRE(node.keys.empty());
        REQUIRE(node.children.empty());
        REQUIRE(node.find_key_index(42) == -1);
    }

    SECTION("Find key index on empty node returns -1") {
        REQUIRE(node.find_key_index(0) == -1);
        REQUIRE(node.find_key_index(100) == -1);
    }
}

TEST_CASE("TrieNode insert_path single element", "[trie_node]") {
    TrieNode root;

    SECTION("Insert single element path") {
        std::vector<id_t> path = {42};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 42);
        REQUIRE(root.children.size() == 1);
        REQUIRE(root.find_key_index(42) == 0);
        REQUIRE(root.find_key_index(43) == -1);
    }

    SECTION("Insert multiple single-element paths") {
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

    SECTION("Insert duplicate single-element path") {
        root.insert_path({42});
        root.insert_path({42});

        // Should not create duplicates
        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 42);
        REQUIRE(root.children.size() == 1);
    }
}

TEST_CASE("TrieNode insert_path multiple elements", "[trie_node]") {
    TrieNode root;

    SECTION("Insert two-element path") {
        std::vector<id_t> path = {10, 20};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 10);
        REQUIRE(root.children.size() == 1);

        TrieNode* child = root.children[0].get();
        REQUIRE(child->keys.size() == 1);
        REQUIRE(child->keys[0] == 20);
        REQUIRE(child->children.size() == 1);
    }

    SECTION("Insert multiple paths with shared prefix") {
        root.insert_path({10, 20});
        root.insert_path({10, 30});
        root.insert_path({15, 25});

        // Root should have keys 10, 15
        REQUIRE(root.keys.size() == 2);
        REQUIRE(root.keys[0] == 10);
        REQUIRE(root.keys[1] == 15);
        REQUIRE(root.children.size() == 2);

        // First child (key 10) should have keys 20, 30
        TrieNode* child_10 = root.children[0].get();
        REQUIRE(child_10->keys.size() == 2);
        REQUIRE(child_10->keys[0] == 20);
        REQUIRE(child_10->keys[1] == 30);
        REQUIRE(child_10->children.size() == 2);

        // Second child (key 15) should have key 25
        TrieNode* child_15 = root.children[1].get();
        REQUIRE(child_15->keys.size() == 1);
        REQUIRE(child_15->keys[0] == 25);
        REQUIRE(child_15->children.size() == 1);
    }

    SECTION("Insert three-element path") {
        std::vector<id_t> path = {1, 2, 3};
        root.insert_path(path);

        REQUIRE(root.keys.size() == 1);
        REQUIRE(root.keys[0] == 1);

        TrieNode* child1 = root.children[0].get();
        REQUIRE(child1->keys.size() == 1);
        REQUIRE(child1->keys[0] == 2);

        TrieNode* child2 = child1->children[0].get();
        REQUIRE(child2->keys.size() == 1);
        REQUIRE(child2->keys[0] == 3);
        REQUIRE(child2->children.size() == 1);
    }

    SECTION("Insert empty path does nothing") {
        std::vector<id_t> empty_path;
        root.insert_path(empty_path);

        REQUIRE(root.keys.empty());
        REQUIRE(root.children.empty());
    }
}

TEST_CASE("TrieIndex navigation", "[trie_index]") {
    TrieNode root;
    root.insert_path({10, 20});
    root.insert_path({10, 30});
    root.insert_path({15, 25});

    SECTION("Basic select operation") {
        TrieIndex index(root);

        // REQUIRE_THROWS( index.select(5) );
        index.select(10);
        // REQUIRE_THROWS( index.select(15) );
    }

    SECTION("Project after select") {
        TrieIndex index(root);

        // Initially at root
        AbstractSet keys = index.project();
        REQUIRE(keys.size() == 2);
        REQUIRE(keys.contains(10));
        REQUIRE(keys.contains(15));

        // Select key 10 and project
        index.select(10);

        keys = index.project();
        REQUIRE(keys.size() == 2);
        REQUIRE(keys.contains(20));
        REQUIRE(keys.contains(30));
    }

    SECTION("Select and backtrack") {
        TrieIndex index(root);

        // Navigate down
        index.select(10);
        AbstractSet child_keys = index.project();
        REQUIRE(child_keys.size() == 2);

        // Backtrack to root
        index.backtrack();
        AbstractSet root_keys = index.project();
        REQUIRE(root_keys.size() == 2);
        REQUIRE(root_keys.contains(10));
        REQUIRE(root_keys.contains(15));
    }

    SECTION("Multiple select operations") {
        TrieIndex index(root);

        index.select(10);
        index.select(20);

        // Should be at leaf node
        AbstractSet leaf_keys = index.project();
        REQUIRE(leaf_keys.empty());

        // Backtrack twice to get to root
        index.backtrack();
        index.backtrack();

        AbstractSet root_keys = index.project();
        REQUIRE(root_keys.size() == 2);
    }
}

TEST_CASE("TrieIndex edge cases", "[trie_index]") {
    SECTION("Empty node trie") {
        TrieNode root;
        TrieIndex index(root);

        AbstractSet keys = index.project();
        REQUIRE(keys.empty());
    }

    SECTION("Trie with only root keys") {
        TrieNode root;
        root.insert_path({10});
        root.insert_path({20});

        TrieIndex index(root);
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

TEST_CASE("TrieNode find_key_index edge cases", "[trie_node]") {
    TrieNode node;

    SECTION("Find in sorted keys") {
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

    SECTION("Find in single-element keys") {
        node.keys = {42};

        REQUIRE(node.find_key_index(42) == 0);
        REQUIRE(node.find_key_index(41) == -1);
        REQUIRE(node.find_key_index(43) == -1);
    }
}
