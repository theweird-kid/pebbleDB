#pragma once

#include "Page.h"

#include <cstdint>
#include <vector>

// Layout of Node

/*
| Field                      | Offset       | Size                                                   |
| -------------------------- | ------------ | ------------------------------------------------------ |
| `NumKeys`                  | 2            | 2 bytes                                                |
| `NextLeaf` (only if leaf)  | 4            | 4 bytes (optional)                                     |
| `Keys[0...n-1]`            | after header | `n * sizeof(int)`                                      |
| `Pointers[0...n]`          | after keys   | for internal: child PageIDs (n+1), for leaf: recordIDs |

*/

constexpr size_t NODE_TYPE_OFFSET      = 0;            // PageType (2 bytes)
constexpr size_t NODE_NUM_KEYS_OFFSET  = 2;            // uint16_t numKeys (2 bytes)
constexpr size_t NODE_NEXT_LEAF_OFFSET = 4;            // uint32_t nextLeaf (4 bytes, only if leaf)
constexpr size_t NODE_HEADER_SIZE_LEAF   = 8;          // 2 + 2 + 4
constexpr size_t NODE_HEADER_SIZE_INTERNAL = 4;        // 2 + 2

static constexpr int MAX_KEYS = 31;
static constexpr int MAX_PTRS_INTERNAL = MAX_KEYS + 1;
static constexpr int MAX_PTRS_LEAF = MAX_KEYS;

class BPlusTreeNode
{
public:
    BPlusTreeNode(Page& page);

    bool isLeaf() const;
    void setLeaf(bool leaf);

    int numKeys() const;
    void setNumKeys(int n);

    int getKey(int idx) const;
    void setKey(int idx, int key);

    uint64_t getPointer(int idx) const;
    void setPointer(int idx, uint64_t ptr);

    uint32_t getNextLeaf() const;
    void setNextLeaf(uint32_t pageID);

private:
    Page& m_Page;

    size_t keyOffset() const;
    size_t ptrOffset() const;
};  