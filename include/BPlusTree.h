#pragma once

#include "BufferPool.h"
#include "BPlusTreeNode.h"

#include <cstdint>
#include <optional>
#include <vector>

class BPlusTree
{
public:
    // Constructor for NEW B+Tree
    BPlusTree(BufferPool& bp, int order = 4)
        : m_BufferPool(bp), m_Order(order)
    {
        m_MinKeys = std::ceil(order / 2.0) - 1;
        m_RootPageID = m_BufferPool.allocatePage();

        Page& page = m_BufferPool.fetchPage(m_RootPageID);
        BPlusTreeNode rootNode(page);
        rootNode.setLeaf(true);

        m_BufferPool.markDirty(m_RootPageID);
        m_BufferPool.unpinPage(m_RootPageID);
    }

    // Constructor for loading an existing B+Tree from a known root
    BPlusTree(BufferPool& bp, uint32_t rootPageID, int order = 4)
        : m_BufferPool(bp), m_Order(order), m_RootPageID(rootPageID)
    {
        m_MinKeys = std::ceil(order / 2.0) - 1;
        // Do NOT allocate a new page
        // Do NOT overwrite the root page
        // Just load the tree rooted at rootPageID
        Page& page = m_BufferPool.fetchPage(m_RootPageID);
        BPlusTreeNode rootNode(page);
        rootNode.setLeaf(true);

        m_BufferPool.markDirty(m_RootPageID);
        m_BufferPool.unpinPage(m_RootPageID);
    }

    bool insert(int key, uint64_t value);                 // Insert key:value pair
    std::optional<uint64_t> search(int key);              // Search for key
    bool update(int key, uint64_t newValue);              // Update value of key
    bool remove(int key);                                 // Remove key

    void print();

    uint32_t rootPageID() const { return m_RootPageID; }

private:
    BufferPool& m_BufferPool;
    uint32_t m_RootPageID;
    int m_Order;                                          // Max keys per node
    int m_MinKeys;                                        // Min keys per node

    // --------------------------------------------- [INSERT] ---------------------------------------------------------
    void insertInternal(int key, uint64_t value, uint32_t pageID,
        int& promotedKey, uint32_t& newChildPageID);

    void splitLeaf(BPlusTreeNode& node, uint32_t pageID, uint32_t& newLeafPageID, int& newKey);
    void splitInternal(BPlusTreeNode& node, uint32_t pageID, uint32_t& newInternalPageID, int& newKey);

    // --------------------------------------------- [REMOVE] ---------------------------------------------------------
    // Recursive helper for remove
    bool removeInternal(int key, PageID nodeID, bool& deleted);

    // Merge helper
    void mergeNodes(PageID leftID, PageID rightID, BPlusTreeNode& parent, int separatorIdx);


    // ---------------------------------------------- [SEARCH] --------------------------------------------------------
    std::optional<uint64_t> searchInternal(int key, uint32_t pageID);

    // ----------------------------------------------- [UPDATE] -------------------------------------------------------
    bool updateInternal(int key, uint64_t newValue, uint32_t pageID);

    // ----------------------------------------------- [PRINT] --------------------------------------------------------
    void printInternal(uint32_t pageID, int level);

    // Utility
    int findChildIndex(BPlusTreeNode& parent, int key);
};
