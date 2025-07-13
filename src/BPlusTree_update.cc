#include "../include/BPlusTree.h"

bool BPlusTree::update(int key, int newValue)
{
    auto node = m_Root;
    while(!node->is_Leaf) {
        int idx = 0;
        while(idx < node->keys.size() && node->keys[idx] <= key) idx++;
        node = node->children[idx];
    }

    for(int idx = 0; idx < node->keys.size(); idx++) {
        if(node->keys[idx] == key) {
            node->values[idx] = newValue;
            return true;
        }
    }

    return false;
}