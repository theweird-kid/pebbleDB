#include "BPlusTree.h"

bool BPlusTree::remove(int key)
{
    if(m_RootPageID == 0) {
        return false; // Tree is empty
	}

	bool underflow = removeInternal(key, m_RootPageID, -1, -1, -1, -1);

	Page& rootPage = m_BufferPool.fetchPage(m_RootPageID);
    BPlusTreeNode rootNode(rootPage);

    if(rootNode.numKeys() == 0 && !rootNode.isLeaf()) {
        PageID newRootID = static_cast<PageID>(rootNode.getPointer(0));
        m_RootPageID = newRootID;
	}

    return true;
}

bool BPlusTree::removeInternal(
    int key, 
    PageID node,
    PageID parent,
    int parentIdx,
	PageID leftSibling,
	PageID rightSibling
)
{

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