#include "BPlusTree.h"

bool BPlusTree::remove(int key)
{
    if(m_RootPageID == INVALID_PAGE) {
        return false; // Tree is empty
	}

    bool deleted{ false };
	bool rootUnderflow = removeInternal(key, m_RootPageID, deleted);

	Page& rootPage = m_BufferPool.fetchPage(m_RootPageID);
    BPlusTreeNode rootNode(rootPage);

    if(rootNode.getNumKeys() == 0) {
        if (!rootNode.isLeaf()) {
            // Promote first child as new root
            PageID newRootID = rootNode.getChild(0);
            m_RootPageID = newRootID;
        }
        else
        {
            // Tree is now empty
            m_RootPageID = INVALID_PAGE;
        }
	}

    m_BufferPool.markDirty(m_RootPageID);
    m_BufferPool.unpinPage(m_RootPageID);

    return deleted;
}

bool BPlusTree::removeInternal(
    int key, 
    PageID nodeID,
    bool& deleted
)
{
    Page& nodePage = m_BufferPool.fetchPage(nodeID);
    BPlusTreeNode node(nodePage);

    // ===== LEAF NODE =====
    if (node.isLeaf())
    {
        int idx = node.findKeyIndex(key);
        if (idx < 0 || node.getKey(idx) != key) {
            deleted = false;
            return false;       // key not found
        }

        node.removeKeyAt(idx);
        node.removeValueAt(idx);

        node.setNumKeys(node.getNumKeys() - 1);
        deleted = true;

        m_BufferPool.markDirty(nodeID);
        m_BufferPool.unpinPage(nodeID);
        return node.getNumKeys() < m_MinKeys;
    }
            
    // ====== INTERNAL NODE ======
    int idx = node.findChildIndex(key);
    PageID childID = node.getChild(idx);
    bool childDeleted = false;
    bool childUnderflow = removeInternal(key, childID, childDeleted);
    deleted = childDeleted;

    if (!childUnderflow)
        return false;

    // Handle Underflow
    PageID leftSiblingID = (idx > 0) ? node.getChild(idx - 1) : INVALID_PAGE;
    PageID rightSiblingID = (idx < node.getNumKeys()) ? node.getChild(idx + 1) : INVALID_PAGE;

    Page& childPage = m_BufferPool.fetchPage(childID);
    BPlusTreeNode childNode(childPage);

    // TRY: Borrow from Left Sibling
    if (leftSiblingID != INVALID_PAGE)
    {
        Page& leftPage = m_BufferPool.fetchPage(leftSiblingID);
        BPlusTreeNode leftNode(leftPage);

        if (leftNode.getNumKeys() > m_MinKeys)
        {
            if (childNode.isLeaf()) {
                // Move Last key from left to front of the child
                int borrowKey = leftNode.getKey(leftNode.getNumKeys() - 1);
                int borrowVal = leftNode.getValue(leftNode.getNumValues() - 1);

                leftNode.removeKeyAt(leftNode.getNumKeys() - 1);
                leftNode.removeValueAt(leftNode.getNumKeys() - 1);
                leftNode.setNumKeys(leftNode.getNumKeys() - 1);

                childNode.insertKeyAt(0, borrowKey);
                childNode.insertValueAt(0, borrowVal);
                childNode.setNumKeys(childNode.getNumKeys() + 1);

                node.setKey(idx - 1, borrowKey);
            }
            else
            {
                int separator = node.getKey(idx - 1);
                int borrowKey = leftNode.getKey(leftNode.getNumKeys() - 1);
                PageID borrowChild = leftNode.getChild(leftNode.getNumKeys());

                leftNode.setNumKeys(leftNode.getNumKeys() - 1);

                childNode.insertKeyAt(0, separator);
                childNode.insertChildAt(0, borrowChild);
                childNode.setNumKeys(childNode.getNumKeys() + 1);

                node.setKey(idx - 1, borrowKey);
            }

            m_BufferPool.markDirty(childID);
            m_BufferPool.markDirty(nodeID);
            m_BufferPool.markDirty(leftSiblingID);
            
            m_BufferPool.unpinPage(childID);
            m_BufferPool.unpinPage(nodeID);
            m_BufferPool.unpinPage(leftSiblingID);

            return false;
        }
    }

    // TRY: Borrow from right Sibling
    if (rightSiblingID != INVALID_PAGE)
    {
        Page& rightPage = m_BufferPool.fetchPage(rightSiblingID);
        BPlusTreeNode rightNode(rightPage);

        if (rightNode.getNumKeys() > m_MinKeys)
        {
            if (childNode.isLeaf())
            {
                int borrowKey = rightNode.getKey(0);
                int borrowVal = rightNode.getValue(0);

                rightNode.removeKeyAt(0);
                rightNode.removeValueAt(0);
                rightNode.setNumKeys(rightNode.getNumKeys() - 1);

                childNode.insertKeyAt(childNode.getNumKeys(), borrowKey);
                childNode.insertValueAt(childNode.getNumKeys(), borrowVal);
                childNode.setNumKeys(childNode.getNumKeys() + 1);

                node.setKey(idx, rightNode.getKey(0));
            }
            else
            {
                int separator = node.getKey(idx);
                int borrowKey = rightNode.getKey(0);
                PageID borrowChild = rightNode.getChild(0);

                rightNode.removeKeyAt(0);
                rightNode.removeChildAt(0);
                rightNode.setNumKeys(rightNode.getNumKeys() - 1);

                childNode.insertKeyAt(childNode.getNumKeys(), separator);
                childNode.insertChildAt(childNode.getNumKeys() + 1, borrowChild);
                childNode.setNumKeys(childNode.getNumKeys() + 1);

                node.setKey(idx, borrowKey);
            }

            m_BufferPool.markDirty(childID);
            m_BufferPool.markDirty(nodeID);
            m_BufferPool.markDirty(rightSiblingID);

            m_BufferPool.unpinPage(childID);
            m_BufferPool.unpinPage(nodeID);
            m_BufferPool.unpinPage(rightSiblingID);

            return false;
        }
    }

    // Merge with Sibling
    if (leftSiblingID != INVALID_PAGE) {
        mergeNodes(leftSiblingID, childID, node, idx - 1);

        m_BufferPool.markDirty(childID);
        m_BufferPool.markDirty(nodeID);
        m_BufferPool.markDirty(leftSiblingID);

        m_BufferPool.unpinPage(childID);
        m_BufferPool.unpinPage(nodeID);
        m_BufferPool.unpinPage(leftSiblingID);

        return node.getNumKeys() < m_MinKeys;
    }
    else if (rightSiblingID != INVALID_PAGE) {
        mergeNodes(childID, rightSiblingID, node, idx);

        m_BufferPool.markDirty(childID);
        m_BufferPool.markDirty(nodeID);
        m_BufferPool.markDirty(rightSiblingID);

        m_BufferPool.unpinPage(childID);
        m_BufferPool.unpinPage(nodeID);
        m_BufferPool.unpinPage(rightSiblingID);

        return node.getNumKeys() < m_MinKeys;
    }

    return false;
}


void BPlusTree::mergeNodes(PageID leftID, PageID rightID, BPlusTreeNode& parent, int separatorIdx) {
    Page& leftPage = m_BufferPool.fetchPage(leftID);
    Page& rightPage = m_BufferPool.fetchPage(rightID);

    BPlusTreeNode left(leftPage);
    BPlusTreeNode right(rightPage);

    if (left.isLeaf()) {
        // Append all keys and values from right to left
        for (int i = 0; i < right.getNumKeys(); ++i) {
            left.insertKeyAt(left.getNumKeys(), right.getKey(i));
            left.insertValueAt(left.getNumKeys(), right.getValue(i));
            left.setNumKeys(left.getNumKeys() + 1);
        }
    }
    else {
        // Append separator key from parent
        left.insertKeyAt(left.getNumKeys(), parent.getKey(separatorIdx));
        left.setNumKeys(left.getNumKeys() + 1);

        // Append all keys and children from right
        for (int i = 0; i < right.getNumKeys(); ++i) {
            left.insertKeyAt(left.getNumKeys(), right.getKey(i));
            left.setNumKeys(left.getNumKeys() + 1);
        }
        for (int i = 0; i <= right.getNumKeys(); ++i) {
            left.insertChildAt(left.getNumKeys(), right.getChild(i));
        }
    }

    // Remove separator from parent
    parent.removeKeyAt(separatorIdx);
    parent.removeChildAt(separatorIdx + 1);
    parent.setNumKeys(parent.getNumKeys() - 1);

    // Mark dirty
    m_BufferPool.markDirty(leftID);
    m_BufferPool.markDirty(rightID);

    // Unpin
    m_BufferPool.unpinPage(leftID);
    m_BufferPool.unpinPage(rightID);

    // Free the right Page
    m_BufferPool.freePage(rightID);
}
