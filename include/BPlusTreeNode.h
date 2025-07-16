#pragma once

#include "Page.h"

#include <cstdint>
#include <vector>

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