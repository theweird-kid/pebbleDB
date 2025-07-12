#pragma once

#include <vector>
#include <memory>
#include <optional>

struct Node {
    bool is_Leaf;
    std::vector<int> keys;
    std::vector<std::shared_ptr<Node>> children;    // Applicable to internal nodes
    std::vector<int> values;                        // Applicable only if Leaf node
    std::shared_ptr<Node> next_Node;                // If Leaf, then pointer to next node

    Node(bool leaf) : is_Leaf(leaf) {}
};

class BPlusTree
{
public:
    BPlusTree(int order = 3);                   // No. of keys per node/page

    void insert(int key, int value);            // Insert new key:value pair
    std::optional<int> search(int key);         // Search key

    void print();

private:
    int m_Order;
    std::shared_ptr<Node> m_Root;

    // helper function for insertion
    void insertInternal(int key, int value, std::shared_ptr<Node> node, std::shared_ptr<Node>& newChild, int& newKey);

    // helper for search
    std::optional<int> searchInternal(int key, std::shared_ptr<Node> node);

    // helper for print
    void printInternal(std::shared_ptr<Node> node, int level);
};