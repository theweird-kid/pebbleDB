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

    bool update(int key, int newValue);         // Update an existing value

    bool remove(int key);                       // Remove key:value pair

    void print();

private:
    int m_Order;
    int m_MinKeys;
    std::shared_ptr<Node> m_Root;

    // --------------------------------------------- HELPER FUNCTIONS -----------------------------------------------

    // --------------------------------------------- [INSERT] ---------------------------------------------------------
    
    void insertInternal(int key, int value, std::shared_ptr<Node> node, std::shared_ptr<Node>& newChild, int& newKey);

    // ----------------------------------------------------------------------------------------------------------------



    // --------------------------------------------- [REMOVE] ---------------------------------------------------------
    
    bool removeInternal(int key, std::shared_ptr<Node> node, std::shared_ptr<Node> parent, int parentIdx,
    std::shared_ptr<Node> leftSibling, std::shared_ptr<Node> rightSibling);

    // Borrow from left sibling if possible
    void borrowFromLeft(std::shared_ptr<Node> node, std::shared_ptr<Node> sibling,
        std::shared_ptr<Node> parent, int parentIdx);

    // Borrow from right sibling if possible
    void borrowFromRight(std::shared_ptr<Node> node, std::shared_ptr<Node> sibling, 
        std::shared_ptr<Node> parent, int parentIdx);

    // Merge with left/right sibling
    void mergeWithSibling(std::shared_ptr<Node> leftNode, std::shared_ptr<Node> rightNode, 
        std::shared_ptr<Node> parent, int parentIdx);

    // ----------------------------------------------------------------------------------------------------------------


    // ---------------------------------------------- [SEARCH] ---------------------------------------------------------
    
    std::optional<int> searchInternal(int key, std::shared_ptr<Node> node);

    // ----------------------------------------------------------------------------------------------------------------


    // ----------------------------------------------- [PRINT] --------------------------------------------------------
    void printInternal(std::shared_ptr<Node> node, int level);
};