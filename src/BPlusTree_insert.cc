#include "BPlusTree.h"

bool BPlusTree::insert(int key, uint64_t value) {
    int promotedKey = -1;
    uint32_t newChildPageID = 0;

    // Duplicate key, insertion failed
    if (search(key).has_value()) {
        return false;  
    }

    insertInternal(key, value, m_RootPageID, promotedKey, newChildPageID);

    if (newChildPageID != 0) {
        uint32_t newRootID = m_BufferPool.allocatePage();
        Page& newRootPage = m_BufferPool.fetchPage(newRootID);
        BPlusTreeNode newRoot(newRootPage);

        newRoot.setLeaf(false);
        newRoot.setNumKeys(1);
        newRoot.setKey(0, promotedKey);
        newRoot.setPointer(0, m_RootPageID);
        newRoot.setPointer(1, newChildPageID);

        m_BufferPool.markDirty(newRootID);
        m_BufferPool.unpinPage(newRootID);

        m_RootPageID = newRootID;
        return true;
    }
	return false;  // No split occurred, tree remains unchanged
}

void BPlusTree::insertInternal(
    int key, uint64_t value, 
    uint32_t pageID,
    int& promotedKey, uint32_t& newChildPageID
) {
    Page& page = m_BufferPool.fetchPage(pageID);
    BPlusTreeNode node(page);

    int n = node.numKeys();
    int idx = 0;
    while (idx < n && node.getKey(idx) < key)
        idx++;

    if (node.isLeaf()) {
        // Shift keys and pointers
        for (int i = n; i > idx; --i) {
            node.setKey(i, node.getKey(i - 1));
            node.setPointer(i, node.getPointer(i - 1));
        }

        node.setKey(idx, key);
        node.setPointer(idx, value);
        node.setNumKeys(n + 1);

        if (node.numKeys() > m_Order) {
            splitLeaf(node, pageID, newChildPageID, promotedKey);
        } else {
            newChildPageID = 0;
        }
    } else {
        uint32_t childID = node.getPointer(idx);
        int childPromotedKey = -1;
        uint32_t childNewPageID = 0;

        insertInternal(key, value, childID, childPromotedKey, childNewPageID);

        if (childNewPageID != 0) {
            // Shift keys and pointers
            for (int i = n; i > idx; --i) {
                node.setKey(i, node.getKey(i - 1));
            }
            for (int i = n + 1; i > idx + 1; --i) {
                node.setPointer(i, node.getPointer(i - 1));
            }

            node.setKey(idx, childPromotedKey);
            node.setPointer(idx + 1, childNewPageID);
            node.setNumKeys(n + 1);

            if (node.numKeys() > m_Order) {
                splitInternal(node, pageID, newChildPageID, promotedKey);
            } else {
                newChildPageID = 0;
            }
        } else {
            newChildPageID = 0;
        }
    }

    m_BufferPool.markDirty(pageID);
    m_BufferPool.unpinPage(pageID);
}

void BPlusTree::splitLeaf(BPlusTreeNode& node, uint32_t pageID,
                          uint32_t& newLeafPageID, int& newKey) {
    int total = node.numKeys();
    int mid = total / 2;

    newLeafPageID = m_BufferPool.allocatePage();
    Page& siblingPage = m_BufferPool.fetchPage(newLeafPageID);
    BPlusTreeNode sibling(siblingPage);
    sibling.setLeaf(true);

    // Move second half to sibling
    int siblingKeys = total - mid;
    sibling.setNumKeys(siblingKeys);
    for (int i = 0; i < siblingKeys; ++i) {
        sibling.setKey(i, node.getKey(mid + i));
        sibling.setPointer(i, node.getPointer(mid + i));
    }

    // Update original node
    node.setNumKeys(mid);
    sibling.setNextLeaf(node.getNextLeaf());
    node.setNextLeaf(newLeafPageID);

    m_BufferPool.markDirty(pageID);

    m_BufferPool.markDirty(newLeafPageID);
    m_BufferPool.unpinPage(newLeafPageID);

    newKey = sibling.getKey(0);  // First key in sibling is promoted
}

void BPlusTree::splitInternal(BPlusTreeNode& node, uint32_t pageID,
                              uint32_t& newPageID, int& newKey) {
    int total = node.numKeys();
    int mid = total / 2;
    newKey = node.getKey(mid);

    newPageID = m_BufferPool.allocatePage();
    Page& siblingPage = m_BufferPool.fetchPage(newPageID);
    BPlusTreeNode sibling(siblingPage);
    sibling.setLeaf(false);

    int rightKeys = total - mid - 1;
    sibling.setNumKeys(rightKeys);

    for (int i = 0; i < rightKeys; ++i) {
        sibling.setKey(i, node.getKey(mid + 1 + i));
        sibling.setPointer(i, node.getPointer(mid + 1 + i));
    }
    sibling.setPointer(rightKeys, node.getPointer(total));

    node.setNumKeys(mid);

    m_BufferPool.markDirty(pageID);

    m_BufferPool.markDirty(newPageID);
    m_BufferPool.unpinPage(newPageID);
}
