#include "../include/BPlusTree.h"

#include <iostream>

void BPlusTree::print() {
    printInternal(m_Root, 0);
}

void BPlusTree::printInternal(std::shared_ptr<Node> node, int level) {
    std::cout << std::string(level * 2, ' ');
    if (node->is_Leaf) {
        std::cout << "Leaf: ";
        for (int k : node->keys) std::cout << k << " ";
        std::cout << "\n";
    } else {
        std::cout << "Internal: ";
        for (int k : node->keys) std::cout << k << " ";
        std::cout << "\n";
        for (auto& child : node->children) {
            printInternal(child, level + 1);
        }
    }
}
