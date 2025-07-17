#include "BPlusTreeNode.h"

#include <cstring>  
#include <stdexcept>

// Layout of Node

/*

| Field                      | Offset       | Size                                                   |
| -------------------------- | ------------ | ------------------------------------------------------ |
| -------------------------- | 0            | 2 bytes                                                |
| `NumKeys`                  | 2            | 2 bytes                                                |
| `NextLeaf` (only if leaf)  | 4            | 4 bytes (optional)                                     |
| `Keys[0...n-1]`            | after header | `n * sizeof(int)`                                      |
| `Pointers[0...n]`          | after keys   | for internal: child PageIDs (n+1), for leaf: recordIDs |

*/

BPlusTreeNode::BPlusTreeNode(Page& page)
    : m_Page(page)
{
    if(m_Page.header()->m_Type != PageType::LEAF && m_Page.header()->m_Type != PageType::INTERNAL)
        throw std::runtime_error("Page type is not a B+Tree node");
}

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
    const char* payload = m_Page.payload();
    return *reinterpret_cast<const uint16_t*>(payload + NODE_NUM_KEYS_OFFSET);
}

void BPlusTreeNode::setNumKeys(int n)
{
    char* payload = m_Page.payload();
    *reinterpret_cast<uint16_t*>(payload + NODE_NUM_KEYS_OFFSET) = static_cast<uint16_t>(n);
}

int BPlusTreeNode::getKey(int idx) const
{
    if(idx < 0 || idx >= numKeys()) 
        throw std::out_of_range("Key index out of range");
    
    const char* payload = m_Page.payload();
    const int* keys = reinterpret_cast<const int*>(payload + keyOffset());
    return keys[idx];
}

void BPlusTreeNode::setKey(int idx, int key) 
{
    if(idx < 0 || idx >= numKeys())
        throw std::out_of_range("Key index out of range");

    char * payload = m_Page.payload();
    int* keys = reinterpret_cast<int*>(payload + keyOffset());
    keys[idx] = key;
}

uint64_t BPlusTreeNode::getPointer(int idx) const 
{
    if(idx < 0 || idx > numKeys())
        throw std::out_of_range("Pointer index out of range");

    const char* payload = m_Page.payload();
    const uint64_t* ptrs = reinterpret_cast<const uint64_t*>(payload + ptrOffset());
    return ptrs[idx];
}

void BPlusTreeNode::setPointer(int idx, uint64_t ptr)
{
    if(idx < 0 || idx > numKeys())
        throw std::out_of_range("Pointer index out of range");

    char* payload = m_Page.payload();
    uint64_t* ptrs = reinterpret_cast<uint64_t*>(payload + ptrOffset());
    ptrs[idx] = ptr;
}

uint32_t BPlusTreeNode::getNextLeaf() const
{
    if(!isLeaf())
        throw std::runtime_error("Not a Leaf node");

    const char* payload = m_Page.payload();
    return *reinterpret_cast<const uint32_t*>(payload + NODE_NEXT_LEAF_OFFSET);
}

void BPlusTreeNode::setNextLeaf(uint32_t pageID)
{
    if(!isLeaf())
       throw std::runtime_error("Not a Leaf node");
    
    char* payload = m_Page.payload();
    *reinterpret_cast<uint32_t*>(payload + NODE_NEXT_LEAF_OFFSET) = pageID;
}

size_t BPlusTreeNode::keyOffset() const
{
    return isLeaf() ? NODE_HEADER_SIZE_LEAF : NODE_HEADER_SIZE_INTERNAL;
}

size_t BPlusTreeNode::ptrOffset() const
{
    return keyOffset() + sizeof(int) * numKeys();
}