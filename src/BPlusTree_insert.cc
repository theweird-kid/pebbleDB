#include "pebble/core/BPlusTree.h"

using namespace pebble::core;

bool BPlusTree::insert(int key, uint64_t value) {
    int promotedKey = -1;
    PageID newChildPageID = 0;

    // Duplicate key, insertion failed
    if (search(key).has_value()) {
        return false;  
    }

    insertInternal(key, value, m_RootPageID, promotedKey, newChildPageID);

    if (newChildPageID != 0) {
        PageID newRootID = m_BufferPool.allocatePage();
        Page& newRootPage = m_BufferPool.fetchPage(newRootID);
        BPlusTreeNode newRoot(newRootPage);

        newRoot.setLeaf(false);
        newRoot.setNumKeys(1);
        newRoot.setKey(0, promotedKey);
        newRoot.setChild(0, m_RootPageID);
        newRoot.setChild(1, newChildPageID);

        m_BufferPool.markDirty(newRootID);
        m_BufferPool.unpinPage(newRootID);

        m_RootPageID = newRootID;
        return true;
    }
	return false;  // No split occurred, tree remains unchanged
}

void BPlusTree::insertInternal(
    int key, uint64_t value, 
    PageID pageID,
    int& promotedKey, PageID& newChildPageID
) {
    Page& page = m_BufferPool.fetchPage(pageID);
    BPlusTreeNode node(page);

    int n = node.getNumKeys();
    int idx = 0;
    while (idx < n && node.getKey(idx) < key)
        idx++;

    if (node.isLeaf()) 
    {
        node.insertKeyAt(idx, key);
        node.insertValueAt(idx, value);
        node.setNumKeys(n + 1);

        if (node.getNumKeys() > m_Order) {
            splitLeaf(node, pageID, newChildPageID, promotedKey);
        } else {
            newChildPageID = 0;
        }
    } else {
        PageID childID = node.getChild(idx);
        int childPromotedKey = -1;
        PageID childNewPageID = 0;

        insertInternal(key, value, childID, childPromotedKey, childNewPageID);

        if (childNewPageID != 0) {
            node.insertKeyAt(idx, childPromotedKey);
            node.insertChildAt(idx + 1, childNewPageID);
            node.setNumKeys(n + 1);

            if (node.getNumKeys() > m_Order) {
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

void BPlusTree::splitLeaf(BPlusTreeNode& node, PageID pageID,
                          PageID& newLeafPageID, int& newKey) {
    int total = node.getNumKeys();
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
        sibling.setValue(i, node.getValue(mid + i));
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

void BPlusTree::splitInternal(BPlusTreeNode& node, PageID pageID,
                              PageID& newPageID, int& newKey) {
    int total = node.getNumKeys();
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
        sibling.setChild(i, node.getChild(mid + 1 + i));
    }
    sibling.setChild(rightKeys, node.getChild(total));

    node.setNumKeys(mid);

    m_BufferPool.markDirty(pageID);

    m_BufferPool.markDirty(newPageID);
    m_BufferPool.unpinPage(newPageID);
}
