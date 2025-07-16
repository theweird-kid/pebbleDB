#include "BPlusTreeNode.h"

#include <cstring>  

// Layout of Node

/*

| Field                      | Offset       | Size                                                   |
| -------------------------- | ------------ | ------------------------------------------------------ |
| `PageType` (leaf/internal) | 0            | 2 bytes                                                |
| `NumKeys`                  | 2            | 2 bytes                                                |
| `NextLeaf` (only if leaf)  | 4            | 4 bytes (optional)                                     |
| `Keys[0...n-1]`            | after header | `n * sizeof(int)`                                      |
| `Pointers[0...n]`          | after keys   | for internal: child PageIDs (n+1), for leaf: recordIDs |

*/

BPlusTreeNode::BPlusTreeNode(Page& page)
    : m_Page(page)
{}

bool BPlusTreeNode::isLeaf() const
{
    return m_Page.header()->m_Type == PageType::LEAF;
}

void BPlusTreeNode::setLeaf(bool leaf) 
{
    m_Page.header()->m_Type = (leaf ? PageType::LEAF : PageType::INTERNAL);
}

int BPlusTreeNode::numKeys() const
{
    return *reinterpret_cast<const uint16_t*>(m_Page.data() + 2);
}