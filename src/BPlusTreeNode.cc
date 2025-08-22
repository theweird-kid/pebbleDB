#include "pebble/core/BPlusTreeNode.h"
#include <cstring>
#include <stdexcept>

using namespace pebble::core;

BPlusTreeNode::BPlusTreeNode(Page& page)
    : m_Page(page) {
}

bool BPlusTreeNode::isLeaf() const {
    return m_Page.header()->m_Type == PageType::LEAF;
}

void BPlusTreeNode::setLeaf(bool leaf) {
    m_Page.header()->m_Type = leaf ? PageType::LEAF : PageType::INTERNAL;
}

int BPlusTreeNode::getNumKeys() const {
    return *reinterpret_cast<const uint16_t*>(m_Page.payload() + NODE_NUM_KEYS_OFFSET);
}

void BPlusTreeNode::setNumKeys(int n) {
    *reinterpret_cast<uint16_t*>(m_Page.payload() + NODE_NUM_KEYS_OFFSET) = static_cast<uint16_t>(n);
}

int BPlusTreeNode::getNumValues() const {
    return isLeaf() ? getNumKeys() : getNumKeys() + 1;
}

int BPlusTreeNode::getKey(int idx) const {
    if (idx < 0 || idx >= getNumKeys()) 
        throw std::out_of_range("Key index out of range");
    int key;
    std::memcpy(&key, m_Page.payload() + keyOffset() + idx * sizeof(int), sizeof(int));
    return key;
}

void BPlusTreeNode::setKey(int idx, int key) {
    if (idx < 0 || idx >= MAX_KEYS) 
        throw std::out_of_range("Key index out of range");
    std::memcpy(m_Page.payload() + keyOffset() + idx * sizeof(int), &key, sizeof(int));
}

int BPlusTreeNode::getNumChildren() const {
    if (isLeaf()) 
        throw std::runtime_error("Leaf nodes do not have children");

    return getNumKeys() + 1;
}

uint64_t BPlusTreeNode::getChild(int idx) const {
    if (isLeaf()) 
        throw std::runtime_error("Leaf nodes do not have children");
    if (idx < 0 || idx >= MAX_CHILDREN) 
        throw std::out_of_range("Child index out of range");

    uint64_t ptr;
    std::memcpy(&ptr, m_Page.payload() + childOffset() + idx * sizeof(uint64_t), sizeof(uint64_t));
    return ptr;
}

void BPlusTreeNode::setChild(int idx, uint64_t ptr) {
    if (isLeaf()) 
        throw std::runtime_error("Leaf nodes do not have children");
    if (idx < 0 || idx >= MAX_CHILDREN) 
        throw std::out_of_range("Child index out of range");

    std::memcpy(m_Page.payload() + childOffset() + idx * sizeof(uint64_t), &ptr, sizeof(uint64_t));
}

void BPlusTreeNode::insertChildAt(int idx, uint64_t ptr) {
    for (int i = getNumKeys(); i >= idx; --i)
        setChild(i + 1, getChild(i));

    setChild(idx, ptr);
}


void BPlusTreeNode::removeChildAt(int idx) {
    for (int i = idx + 1; i <= getNumKeys(); ++i)
        setChild(i - 1, getChild(i));

    setChild(getNumKeys(), 0);
}


uint64_t BPlusTreeNode::getValue(int idx) const {
    if (!isLeaf()) throw std::runtime_error("Only leaf nodes hold values");
    if (idx < 0 || idx >= MAX_VALUES) throw std::out_of_range("Value index out of range");
    uint64_t val;
    std::memcpy(&val, m_Page.payload() + childOffset() + idx * sizeof(uint64_t), sizeof(uint64_t));
    return val;
}

void BPlusTreeNode::setValue(int idx, uint64_t value) {
    if (!isLeaf()) throw std::runtime_error("Only leaf nodes hold values");
    if (idx < 0 || idx >= MAX_VALUES) throw std::out_of_range("Value index out of range");
    std::memcpy(m_Page.payload() + childOffset() + idx * sizeof(uint64_t), &value, sizeof(uint64_t));
}

void BPlusTreeNode::insertValueAt(int idx, uint64_t value) {
    for (int i = getNumKeys() - 1; i >= idx; --i)
        setValue(i + 1, getValue(i));
    setValue(idx, value);
}

void BPlusTreeNode::removeValueAt(int idx) {
    for (int i = idx + 1; i < getNumKeys(); ++i)
        setValue(i - 1, getValue(i));

    setValue(getNumKeys() - 1, 0);
}


uint32_t BPlusTreeNode::getNextLeaf() const {
    if (!isLeaf()) throw std::runtime_error("Not a leaf node");
    return *reinterpret_cast<const uint32_t*>(m_Page.payload() + NODE_NEXT_LEAF_OFFSET);
}

void BPlusTreeNode::setNextLeaf(uint32_t pageID) {
    if (!isLeaf()) throw std::runtime_error("Not a leaf node");
    *reinterpret_cast<uint32_t*>(m_Page.payload() + NODE_NEXT_LEAF_OFFSET) = pageID;
}

int BPlusTreeNode::findKeyIndex(int key) const {
    for (int i = 0; i < getNumKeys(); ++i)
        if (getKey(i) == key) return i;
    return -1;
}

int BPlusTreeNode::findChildIndex(int key) const {
    int i = 0;
    while (i < getNumKeys() && key >= getKey(i)) {
        ++i;
    }
    return i; // returns index of child to follow
}


void BPlusTreeNode::insertKeyAt(int idx, int key) {
    for (int i = getNumKeys() - 1; i >= idx; --i)
        setKey(i + 1, getKey(i));

    setKey(idx, key);
}


void BPlusTreeNode::removeKeyAt(int idx) {
    int n = getNumKeys();
    for (int i = idx + 1; i < n; ++i) setKey(i - 1, getKey(i));
}

size_t BPlusTreeNode::keyOffset() const {
    return isLeaf() ? NODE_HEADER_SIZE_LEAF : NODE_HEADER_SIZE_INTERNAL;
}

size_t BPlusTreeNode::childOffset() const {
    return keyOffset() + sizeof(int) * MAX_KEYS;
}
