#include "../include/BPlusTree.h"


bool BPlusTree::remove(int key)
{
    if(m_Root == nullptr) 
        return false;
    
    bool underflow = removeInternal(key, m_Root, nullptr, -1, nullptr, nullptr);

    if(m_Root->keys.empty()) {
        if(!m_Root->is_Leaf) 
            m_Root = m_Root->children[0];   // shrink height
        else
            m_Root = nullptr;              // tree is empty now
    }

    return true;
}

bool BPlusTree::removeInternal(
    int key,
    std::shared_ptr<Node> node,
    std::shared_ptr<Node> parent,
    int parentIdx,
    std::shared_ptr<Node> leftSibling,
    std::shared_ptr<Node> rightSibling
)
{
    // If node is leaf
    if(node->is_Leaf)
    {
        // Find and remove the key:value pair
        auto it = std::find(node->keys.begin(), node->keys.end(), key);
        if(it == node->keys.end()) return false;              // No changes

        int idx = it - node->keys.begin();
        node->keys.erase(it);                                 // erase key
        node->values.erase(node->values.begin() + idx);       // erase value

        // Check and rebalance if required
        if(node->keys.size() >= m_MinKeys)
            return false;                                     // No Underflow
        
        // Borrow from left Sibling
        if(leftSibling && leftSibling->keys.size() > m_MinKeys) {
            borrowFromLeft(node, leftSibling, parent, parentIdx);
            return false;
        }

        // Borrow from right Sibling
        if(rightSibling && rightSibling->keys.size() > m_MinKeys) {
            borrowFromRight(node, rightSibling, parent, parentIdx);
            return false;
        }

        // MERGE if can't borrow
        if(leftSibling)
        {
            mergeWithSibling(leftSibling, node, parent, parentIdx-1);
        }
        else if(rightSibling)
        {
            mergeWithSibling(node, rightSibling, parent, parentIdx);
        }

        return true;                                                // Underflow propagated upwards
    }
    else            // Internal Node
    {
        // Find the idx where key occurs
        int idx = 0;
        while(idx < node->keys.size() && node->keys[idx] <= key) {
            idx++;
        }

        // Search in the child
        auto child = node->children[idx];
        auto leftChild = (idx > 0) ? node->children[idx-1] : nullptr;
        auto rightChild = (idx+1 < node->children.size()) ? node->children[idx+1] : nullptr;

        bool childUnderflow = removeInternal(key, child, node, idx, leftChild, rightChild);

         // After child deletion, check if **this node** now underflows
        if(node->keys.size() >= m_MinKeys) {
            return false; // this node is still OK
        }

        // If this node underflows → try to fix
        if(leftSibling && leftSibling->keys.size() > m_MinKeys) {
            borrowFromLeft(node, leftSibling, parent, parentIdx);
            return false;
        }

        if (rightSibling && rightSibling->keys.size() > m_MinKeys) {
            borrowFromRight(node, rightSibling, parent, parentIdx);
            return false;
        }

        // If can't borrow, merge with sibling
        if (leftSibling) {
            mergeWithSibling(leftSibling, node, parent, parentIdx - 1);
        } else if (rightSibling) {
            mergeWithSibling(node, rightSibling, parent, parentIdx);
        }

        // After merge, propagate underflow upwards
        return true;
    }
}

void BPlusTree::borrowFromLeft(
    std::shared_ptr<Node> node, 
    std::shared_ptr<Node> leftSibling, 
    std::shared_ptr<Node> parent, 
    int parentIdx
)
{
    if(node->is_Leaf)
    {
        // Move the largest key:value from left Sibling to node
        node->keys.insert(node->keys.begin(), leftSibling->keys.back());
        leftSibling->keys.pop_back();
        // Move value
        node->values.insert(node->values.begin(), leftSibling->values.back());
        leftSibling->values.pop_back();

        // Update parent's separator key
        parent->keys[parentIdx-1] = node->keys[0];
    }
    else    // Internal Node
    {
        // Bring down parent's separator key
        int parentKey = parent->keys[parentIdx-1];
        node->keys.insert(node->keys.begin(), parentKey);

        // Bring over the child pointer from left Sibling
        node->children.insert(node->children.begin(), leftSibling->children.back());
        leftSibling->children.pop_back();

        // Move up left sibling's largest key to parent
        parent->keys[parentIdx-1] = leftSibling->keys.back();
        leftSibling->keys.pop_back();
    }
}

void BPlusTree::borrowFromRight(
    std::shared_ptr<Node> node, 
    std::shared_ptr<Node> rightSibling, 
    std::shared_ptr<Node> parent, 
    int parentIdx
)
{
    if(node->is_Leaf)
    {
        // Move the smallest key:value pair from right Sibling to node
        node->keys.insert(node->keys.end(), rightSibling->keys.front());
        rightSibling->keys.erase(rightSibling->keys.begin());
        // Move value
        node->values.insert(node->values.end(), rightSibling->values.front());
        rightSibling->values.erase(rightSibling->values.begin());

        // Update the parent's separator key
        parent->keys[parentIdx] = rightSibling->keys.front();
    }
    else    // Internal Node
    {
        // Bring down parent's separator key
        int parentKey = parent->keys[parentIdx];
        node->keys.insert(node->keys.end(), parentKey);

        // Bring over the children from right Sibling
        node->children.insert(node->children.end(), rightSibling->children.front());
        rightSibling->children.erase(rightSibling->children.begin());

        // Move up right sibling's smallest key to parent
        parent->keys[parentIdx] = rightSibling->keys.front();
        rightSibling->keys.erase(rightSibling->keys.begin());
    }
}

// We will merge right Node into left node and delete the right node
// opposite can also be done
void BPlusTree::mergeWithSibling(
    std::shared_ptr<Node> leftNode, 
    std::shared_ptr<Node> rightNode, 
    std::shared_ptr<Node> parent, 
    int parentIdx
)
{   
    if(leftNode->is_Leaf)   // Can check any left/right
    {
        // Merge key:value pairs from right node to left node
        leftNode->keys.insert(leftNode->keys.end(), rightNode->keys.begin(), rightNode->keys.end());
        leftNode->values.insert(leftNode->values.end(), rightNode->values.begin(), rightNode->values.end());

        // remove the right node and adjust the link
        leftNode->next_Node = rightNode->next_Node;
    }
    else    // Internal Node
    {
        // Bring down the parent's separator key
        int parentKey = parent->keys[parentIdx];
        leftNode->keys.push_back(parentKey);

        // Merge keys and children from right node to left node
        leftNode->keys.insert(leftNode->keys.end(), rightNode->keys.begin(), rightNode->keys.end());
        leftNode->children.insert(leftNode->children.end(), rightNode->children.begin(), rightNode->children.end());
    }

    // remove the parent's separator key and it's children to right
    parent->keys.erase(parent->keys.begin() + parentIdx);
    parent->children.erase(parent->children.begin() + parentIdx+1);
}
