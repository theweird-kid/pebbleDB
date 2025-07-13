#include "../include/BPlusTree.h"

// Insert function
// Starts recursive insertion, and if root splits, creates a new root.
void BPlusTree::insert(int key, int value)
{
    std::shared_ptr<Node> newChild = nullptr; // holds new child if split happens
    int newKey = 0;                           // key to promote if split happens

    insertInternal(key, value, m_Root, newChild, newKey);

    // If root was split, create a new root and point to both children
    if (newChild)
    {
        auto newRoot = std::make_shared<Node>(false); // new root is internal
        newRoot->keys.push_back(newKey);
        newRoot->children.push_back(m_Root);
        newRoot->children.push_back(newChild);
        m_Root = newRoot;
    }
}

// [HELPER] Recursive insertion
// Handles both leaf and internal nodes. Splits nodes when needed.
void BPlusTree::insertInternal(
    int key, int value,
    std::shared_ptr<Node> node,
    std::shared_ptr<Node>& newChild,
    int& newKey
)
{
    // Find position to insert or descend
    int idx = 0;
    while (idx < node->keys.size() && node->keys[idx] < key) {
        idx++;
    }

    // --- Leaf node case ---
    if (node->is_Leaf)
    {
        // Insert key and value at found position
        node->keys.insert(node->keys.begin() + idx, key);
        node->values.insert(node->values.begin() + idx, value);

        // If node does NOT overflow
        if (node->keys.size() < m_Order) {
            newChild = nullptr; // no split
        }
        else
        {
            // Split leaf node
            auto sibling = std::make_shared<Node>(true); // new right sibling
            int mid = (m_Order / 2);

            // Move second half of keys & values to sibling
            sibling->keys.assign(node->keys.begin() + mid, node->keys.end());
            sibling->values.assign(node->values.begin() + mid, node->values.end());

            // Resize current (left) node
            node->keys.resize(mid);
            node->values.resize(mid);

            // Update linked list of leaves
            sibling->next_Node = node->next_Node;
            node->next_Node = sibling;

            // Promote smallest key of sibling
            newKey = sibling->keys[0];
            newChild = sibling;
        }
    }

    // --- Internal node case ---
    else
    {
        std::shared_ptr<Node> childNewChild = nullptr;
        int childNewKey = 0;

        // Recurse into child
        insertInternal(key, value, node->children[idx], childNewChild, childNewKey);

        if (childNewChild)
        {
            // Insert promoted key & child pointer into this internal node
            node->keys.insert(node->keys.begin() + idx, childNewKey);
            node->children.insert(node->children.begin() + idx + 1, childNewChild);

            // If this internal node overflows
            if (node->keys.size() >= m_Order)
            {
                auto sibling = std::make_shared<Node>(false); // new right sibling
                int mid = (m_Order / 2);

                // Right sibling takes keys & children from mid+1 onward
                sibling->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
                sibling->children.assign(node->children.begin() + mid + 1, node->children.end());

                // Left node shrinks to mid keys & mid+1 children
                node->keys.resize(mid);
                node->children.resize(mid + 1);

                newChild = sibling;
                // Promote middle key
                newKey = node->keys[mid];
            }
            else
            {
                newChild = nullptr; // no split
            }
        }
    }
}
