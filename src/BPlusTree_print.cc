#include "BPlusTree.h"
#include <queue>
#include <iostream>

void BPlusTree::print() {
    if (m_RootPageID == 0) {
        std::cout << "Empty B+ Tree\n";
        return;
    }

    std::queue<uint32_t> q;
    q.push(m_RootPageID);
    int level = 0;

    std::cout << "B+ Tree Structure:\n";

    while (!q.empty()) {
        int sz = q.size();
        std::cout << "Level " << level << ":\n";

        for (int i = 0; i < sz; ++i) {
            uint32_t pageID = q.front();
            q.pop();

            Page& page = m_BufferPool.fetchPage(pageID);
            BPlusTreeNode node(page);

            int n = node.numKeys();
            std::cout << "[Page " << pageID << "] ";

            std::cout << (node.isLeaf() ? "(Leaf) " : "(Internal) ");
            for (int j = 0; j < n; ++j) {
                std::cout << node.getKey(j);
                if (j < n - 1) std::cout << ", ";
            }
            std::cout << "  ";

            if (!node.isLeaf()) {
                // Push child pointers to queue
                for (int j = 0; j <= n; ++j) {
                    q.push(node.getPointer(j));
                }
            }

            m_BufferPool.unpinPage(pageID);
        }

        std::cout << "\n";
        ++level;
    }

    std::cout << "End of Tree\n";
}
