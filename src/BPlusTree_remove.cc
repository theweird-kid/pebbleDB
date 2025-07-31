#include "BPlusTree.h"

bool BPlusTree::remove(int key)
{
    if(m_RootPageID == 0) {
        return false; // Tree is empty
	}

	bool underflow = removeInternal(key, m_RootPageID, -1, -1, -1, -1);

	Page& rootPage = m_BufferPool.fetchPage(m_RootPageID);
    BPlusTreeNode rootNode(rootPage);

    if(rootNode.getNumKeys() == 0 && !rootNode.isLeaf()) {
        PageID newRootID = static_cast<PageID>(rootNode.getPointer(0));
        m_RootPageID = newRootID;
	}

    m_BufferPool.unpinPage(m_RootPageID);
    return true;
}

bool BPlusTree::removeInternal(
    int key, 
    PageID nodeID,
    PageID parentID,
    int parentIdx,
	PageID leftSiblingID,
	PageID rightSiblingID
)
{
    Page& nodePage = m_BufferPool.fetchPage(nodeID);
    BPlusTreeNode node(nodePage);

    // ===== LEAF NODE =====
    if (node.isLeaf())
    {
        // Find and remove the key:value pair
        int idx = node.findKeyIndex(key);
        if (idx == -1) return false;        // No changes

        node.removeKeyAt(idx);      // erase key
        node.removeValueAt(idx);    // erase value

        // Check and rebalance if required
        if (node.getNumKeys() >= m_MinKeys)
            return false;

        // Borrow from left Sibling
        if (leftSiblingID != -1) {
            Page& leftPage = m_BufferPool.fetchPage(leftSiblingID); // [REMEMBER] markDirty and unpin
            BPlusTreeNode leftSibling(leftPage);
            if (leftSibling.getNumKeys() > m_MinKeys) {
                borrowFromLeft(nodeID, leftSiblingID, parentID, parentIdx);
                m_BufferPool.markDirty(leftSiblingID);
                m_BufferPool.unpinPage(leftSiblingID);
                return false;
            }
            m_BufferPool.unpinPage(leftSiblingID);
        }

        // Borrow from right Sibling
        if (rightSiblingID != -1) {
            Page& rightPage = m_BufferPool.fetchPage(rightSiblingID);   // [REMEMBER] markDirty and unpin
            BPlusTreeNode rightSibling(rightPage);
            if (rightSibling.getNumKeys() > m_MinKeys) {
                borrowFromRight(nodeID, rightSiblingID, parentID, parentIdx);
                m_BufferPool.markDirty(rightSiblingID);
                m_BufferPool.unpinPage(rightSiblingID);
                return false;
            }
            m_BufferPool.unpinPage(rightSiblingID);
        }

        // MERGE if can't borrow
        if (leftSiblingID != -1)
        {
            mergeWithSibling(leftSiblingID, nodeID, parentID, parentIdx-1);
        }
        else if (rightSiblingID != -1)
        {
            mergeWithSibling(nodeID, rightSiblingID, parentID, parentIdx);
        }

        m_BufferPool.markDirty(nodeID);
        m_BufferPool.unpinPage(nodeID);

        return true;
    }
    else  
    {
        // ===== INTERNAL NODE =====
        
        // Find the idx where key occurs
        int idx = 0;
        while (idx < node.getNumKeys() && node.getKey(idx) <= key) {
            idx++;
        }

        // Search in the child
        PageID childID = node.getPointer(idx);
        PageID leftChildID = (idx > 0) ? node.getPointer(idx - 1) : -1;
        PageID rightChildID = (idx + 1 < node.getNumValues()) ? node.getPointer(idx + 1) : -1;

        bool childUnderflow = removeInternal(key, childID, nodeID, idx, leftChildID, rightChildID);

        // After child deletion, check if *this* node now underflows
        if (node.getNumKeys() >= m_MinKeys) {
            return false;   // this node is still ok
        }

        // If this node underflows -> try to fix
        if (leftSiblingID != -1) {
            Page& leftPage = m_BufferPool.fetchPage(leftSiblingID); // [REMEMBER] markDirty and unpin
            BPlusTreeNode leftSibling(leftPage);
            if (leftSibling.getNumKeys() > m_MinKeys) {
                borrowFromLeft(nodeID, leftSiblingID, parentID, parentIdx);
                m_BufferPool.markDirty(leftSiblingID);
                m_BufferPool.unpinPage(leftSiblingID);
                return false;
            }
            m_BufferPool.unpinPage(leftSiblingID);
        }

        if (rightSiblingID != -1) {
            Page& rightPage = m_BufferPool.fetchPage(rightSiblingID);   // [REMEMBER] markDirty and unpin
            BPlusTreeNode rightSibling(rightPage);
            if (rightSibling.getNumKeys() > m_MinKeys) {
                borrowFromRight(nodeID, rightSiblingID, parentID, parentIdx);
                m_BufferPool.markDirty(rightSiblingID);
                m_BufferPool.unpinPage(rightSiblingID);
                return false;
            }
            m_BufferPool.unpinPage(rightSiblingID);
        }

        // MERGE if can't borrow
        if (leftSiblingID != -1)
        {
            mergeWithSibling(leftSiblingID, nodeID, parentID, parentIdx-1);
        }
        else if (rightSiblingID != -1)
        {
            mergeWithSibling(nodeID, rightSiblingID, parentID, parentIdx);
        }

        m_BufferPool.markDirty(nodeID);
        m_BufferPool.unpinPage(nodeID);

        // propagate underflow
        return true;
    }
}

void BPlusTree::borrowFromLeft(
    PageID node, 
    PageID leftSibling,
    PageID parent, int parentIdx
)
{

}

void BPlusTree::borrowFromRight(
    PageID node, 
    PageID rightSibling,
    PageID parent, int parentIdx
)
{

}

void BPlusTree::mergeWithSibling(
    PageID leftNode,
    PageID rightNode,
    PageID parent, int parentIdx
)
{

}