#pragma once

#include "pebble/core/Page.h"

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

namespace pebble {
    namespace core {

        constexpr size_t NODE_TYPE_OFFSET = 0;            // PageType (2 bytes)
        constexpr size_t NODE_NUM_KEYS_OFFSET = 2;            // uint16_t numKeys (2 bytes)
        constexpr size_t NODE_NEXT_LEAF_OFFSET = 4;            // uint32_t nextLeaf (4 bytes, only if leaf)
        constexpr size_t NODE_HEADER_SIZE_LEAF = 8;            // 2 + 2 + 4
        constexpr size_t NODE_HEADER_SIZE_INTERNAL = 4;            // 2 + 2

        static constexpr int MAX_KEYS = 31;
        static constexpr int MAX_CHILDREN = MAX_KEYS + 1;
        static constexpr int MAX_VALUES = MAX_KEYS;

        constexpr PageID INVALID_PAGE = -1;

        class BPlusTreeNode {
        public:
            explicit BPlusTreeNode(Page& page);

            // Node type
            bool isLeaf() const;
            void setLeaf(bool isLeaf);

            // Keys
            int getNumKeys() const;
            void setNumKeys(int n);
            int getKey(int idx) const;
            void setKey(int idx, int key);

            int findKeyIndex(int key) const;
            int findChildIndex(int key) const;

            void insertKeyAt(int idx, int key);
            void removeKeyAt(int idx);

            // ====== LEAF NODE API ======
            int getNumValues() const;
            uint64_t getValue(int idx) const;
            void setValue(int idx, uint64_t value);
            void insertValueAt(int idx, uint64_t value);
            void removeValueAt(int idx);

            uint32_t getNextLeaf() const;
            void setNextLeaf(uint32_t pageID);

            // ====== INTERNAL NODE API ======
            int getNumChildren() const; // equals getNumKeys() + 1
            uint64_t getChild(int idx) const;
            void setChild(int idx, uint64_t pageID);
            void insertChildAt(int idx, uint64_t pageID);
            void removeChildAt(int idx);

        private:
            Page& m_Page;

            size_t keyOffset() const;
            size_t childOffset() const;
        };
    }
}