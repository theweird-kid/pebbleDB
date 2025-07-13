#include "../include/BPlusTree.h"

#include <iostream>
#include <algorithm>

BPlusTree::BPlusTree(int order) : m_Order(order)
{
    m_Root = std::make_shared<Node>(true);
    m_MinKeys = std::ceil(order/2.0) - 1;
}

std::optional<int> BPlusTree::search(int key)
{
    return searchInternal(key, m_Root);
}

std::optional<int> BPlusTree::searchInternal(int key, std::shared_ptr<Node> node)
{   
    // Locate the required key' index on the current Node
    int idx = 0;
    while(idx < node->keys.size() && node->keys[idx] < key) {
        idx++;
    }

    // If node is leaf then we found the value
    if(node->is_Leaf) 
    {
        if(idx < node->keys.size() && node->keys[idx] == key) {
            return node->values[idx];
        }
        else {
            return std::nullopt;
        }
    }
    else {  // Else search in children till we reach a leaf node
        return searchInternal(key, node->children[idx]);
    }
}