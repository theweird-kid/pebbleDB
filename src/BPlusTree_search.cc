#include "pebble/core/BPlusTree.h"
#include <iostream>

using namespace pebble::core;

std::optional<uint64_t> BPlusTree::search(int key)
{
    return searchInternal(key, m_RootPageID);
}

std::optional<uint64_t> BPlusTree::searchInternal(int key, uint32_t pageID)
{
    Page& page = m_BufferPool.fetchPage(pageID);
    BPlusTreeNode node(page);

    int n = node.getNumKeys();

    if (node.isLeaf())
    {

        for (int i = 0; i < n; ++i) {
            if (node.getKey(i) == key) {
                return node.getValue(i);  // pointer = value (record ID)
            }
        }

        return std::nullopt;  // not found in leaf
    }
    else
    {
        // Internal: find correct child to descend into
        int idx = 0;
        while (idx < n && key >= node.getKey(idx)) {
            idx++;
        }

        // Descend into child at index `idx`
        return searchInternal(key, node.getChild(idx));
    }
}
