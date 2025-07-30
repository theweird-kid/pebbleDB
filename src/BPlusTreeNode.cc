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

int BPlusTreeNode::getKey(int idx) const {
    if (idx < 0 || idx >= numKeys())  // You only read valid keys
        throw std::out_of_range("Key index out of range");

    int key;
    std::memcpy(&key, m_Page.payload() + keyOffset() + idx * sizeof(int), sizeof(int));
    return key;
}

void BPlusTreeNode::setKey(int idx, int key) {
    // You can set at most MAX_KEYS keys. During insert idx may equal numKeys().
    if (idx < 0 || idx >= MAX_KEYS)
        throw std::out_of_range("Key index out of range");

    std::memcpy(m_Page.payload() + keyOffset() + idx * sizeof(int), &key, sizeof(int));
}

uint64_t BPlusTreeNode::getPointer(int idx) const {
    int maxPtrs = isLeaf() ? MAX_PTRS_LEAF : MAX_PTRS_INTERNAL;

    if (idx < 0 || idx >= maxPtrs)
        throw std::out_of_range("Pointer index out of range");

    uint64_t ptr;
    std::memcpy(&ptr, m_Page.payload() + ptrOffset() + idx * sizeof(uint64_t), sizeof(uint64_t));
    return ptr;
}

void BPlusTreeNode::setPointer(int idx, uint64_t ptr) {
    int maxPtrs = isLeaf() ? MAX_PTRS_LEAF : MAX_PTRS_INTERNAL;

    if (idx < 0 || idx >= maxPtrs)
        throw std::out_of_range("Pointer index out of range");

    std::memcpy(m_Page.payload() + ptrOffset() + idx * sizeof(uint64_t), &ptr, sizeof(uint64_t));
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
    return keyOffset() + sizeof(int) * MAX_KEYS;
}