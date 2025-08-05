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
        PageID newRootID = static_cast<PageID>(rootNode.getChild(0));
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
        if (node.getNumKeys() >= m_MinKeys) {
            m_BufferPool.unpinPage(nodeID);
            return false;
        }
            
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
        PageID childID = node.getChild(idx);
        PageID leftChildID = (idx > 0) ? node.getChild(idx - 1) : -1;
        PageID rightChildID = (idx + 1 < node.getNumValues()) ? node.getChild(idx + 1) : -1;

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
    PageID nodeID, 
    PageID leftSiblingID,
    PageID parentID, int parentIdx
)
{
    Page& nodePage = m_BufferPool.fetchPage(nodeID);
    BPlusTreeNode node(nodePage);

    Page& leftPage = m_BufferPool.fetchPage(leftSiblingID);
    BPlusTreeNode leftSibling(leftPage);

    Page& parentPage = m_BufferPool.fetchPage(parentID);
    BPlusTreeNode parent(parentPage);

    if (node.isLeaf())
    {
        // Move the largest key:value from left sibling to node
        int borrowedKey = leftSibling.getKey(leftSibling.getNumKeys() - 1);
        int borrowedValue = leftSibling.getValue(leftSibling.getNumValues() - 1); 

        // Remove from left Sibling
        leftSibling.removeKeyAt(leftSibling.getNumKeys() - 1);
        leftSibling.removeValueAt(leftSibling.getNumValues() - 1);

        // Insert at front of node
        node.insertKeyAt(0, borrowedKey);
        node.insertValueAt(0, borrowedValue);

        // Update Parent's separator key
        parent.setKey(parentIdx - 1, node.getKey(0));
    }
    else
    {
        // Bring down parent's separator key
        int parentKey = parent.getKey(parentIdx - 1);
        node.insertKeyAt(0, parentKey);

        // Move last child pointer from left Sibling to front of node
        uint64_t borrowedChild = leftSibling.getChild(leftSibling.getNumKeys());
        node.insertChildAt(0, borrowedChild);

        // update parent key with left sibling's last key
        int newParentKey = leftSibling.getKey(leftSibling.getNumKeys() - 1);
        parent.setKey(parentIdx - 1, newParentKey);

        // Remove from left sibling
        leftSibling.removeKeyAt(leftSibling.getNumKeys() - 1);
        leftSibling.removeChildAt(leftSibling.getNumValues()-1);
    }

    m_BufferPool.markDirty(nodeID);
    m_BufferPool.markDirty(leftSiblingID);
    m_BufferPool.markDirty(parentID);

    m_BufferPool.unpinPage(nodeID);
    m_BufferPool.unpinPage(leftSiblingID);
    m_BufferPool.unpinPage(parentID);
}

void BPlusTree::borrowFromRight(
    PageID nodeID, 
    PageID rightSiblingID,
    PageID parentID, int parentIdx
)
{
    Page& nodePage = m_BufferPool.fetchPage(nodeID);
    BPlusTreeNode node(nodePage);

    Page& rightPage = m_BufferPool.fetchPage(rightSiblingID);
    BPlusTreeNode rightSibling(rightPage);

    Page& parentPage = m_BufferPool.fetchPage(parentID);
    BPlusTreeNode parent(parentPage);

    if (node.isLeaf())
    {
        // Move the smallest key:value pair from right Sibling to node
        int borrowedKey = rightSibling.getKey(0);
        int borrowedValue = rightSibling.getValue(0);

        // Remove from right Sibling
        rightSibling.removeKeyAt(0);
        rightSibling.removeValueAt(0);

        // Insert at the end of the node
        node.insertKeyAt(node.getNumKeys(), borrowedKey);
        node.insertValueAt(node.getNumValues(), borrowedValue);

        // Update Parent's separator key to new first key of right Sibling
        if (rightSibling.getNumKeys() > 0)
            parent.setKey(parentIdx, rightSibling.getKey(0));
    }
    else
    {
        // Bring down parent's separator key into node
        int parentKey = parent.getKey(parentIdx);
        node.insertKeyAt(node.getNumKeys(), parentKey);

        // Bring over the first child pointer from right Sibling
        uint64_t borrowedChild = rightSibling.getChild(0);
        node.insertChildAt(node.getNumValues(), borrowedChild);

        // Update parent's key with right Sibling's first key
        int newParentKey = rightSibling.getKey(0);
        parent.setKey(parentIdx, newParentKey);

        // Remove borrowed key and pointer from right sibling
        rightSibling.removeKeyAt(0);
        rightSibling.removeChildAt(0);
    }

    m_BufferPool.markDirty(nodeID);
    m_BufferPool.markDirty(rightSiblingID);
    m_BufferPool.markDirty(parentID);

    m_BufferPool.unpinPage(nodeID);
    m_BufferPool.unpinPage(rightSiblingID);
    m_BufferPool.unpinPage(parentID);
}

void BPlusTree::mergeWithSibling(
    PageID leftNodeID,
    PageID rightNodeID,
    PageID parentID, int parentIdx
) {
    Page& leftPage = m_BufferPool.fetchPage(leftNodeID);
    BPlusTreeNode leftNode(leftPage);

    Page& rightPage = m_BufferPool.fetchPage(rightNodeID);
    BPlusTreeNode rightNode(rightPage);

    Page& parentPage = m_BufferPool.fetchPage(parentID);
    BPlusTreeNode parent(parentPage);

    if (leftNode.isLeaf()) {
        // Move all key-value pairs from right to left
        for (int i = 0; i < rightNode.getNumKeys(); ++i) {
            int key = rightNode.getKey(i);
            uint64_t val = rightNode.getValue(i);

            leftNode.insertKeyAt(leftNode.getNumKeys(), key);
            leftNode.insertValueAt(leftNode.getNumValues(), val);
        }

        // Update leaf link pointer
        leftNode.setNextLeaf(rightNode.getNextLeaf());

        // Remove separator key and rightNode pointer from parent
        parent.removeKeyAt(parentIdx);
        parent.removeChildAt(parentIdx + 1);

        // TODO: deallocate rightNodeID using a freelist
    }
    else {
        // Insert parent separator key to left node
        int sepKey = parent.getKey(parentIdx);
        leftNode.insertKeyAt(leftNode.getNumKeys(), sepKey);

        // Move all keys and children from right to left
        int rightKeys = rightNode.getNumKeys();
        for (int i = 0; i < rightKeys; ++i) {
            leftNode.insertKeyAt(leftNode.getNumKeys(), rightNode.getKey(i));
        }

        for (int i = 0; i < rightNode.getNumValues(); ++i) {
            leftNode.insertChildAt(leftNode.getNumValues(), rightNode.getChild(i));
        }

        // Remove separator key and rightNode pointer from parent
        parent.removeKeyAt(parentIdx);
        parent.removeChildAt(parentIdx + 1);

        // TODO: deallocate rightNodeID
    }
    m_BufferPool.markDirty(leftNodeID);
    m_BufferPool.markDirty(rightNodeID);
    m_BufferPool.markDirty(parentID);

    m_BufferPool.unpinPage(leftNodeID);
    m_BufferPool.unpinPage(rightNodeID);
    m_BufferPool.unpinPage(parentID);
}
