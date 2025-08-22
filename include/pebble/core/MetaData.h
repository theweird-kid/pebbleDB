#pragma once

#include <cstdint>

namespace pebble {
    namespace core {

        struct MetaData {
            uint32_t m_NextPageID;     // Next page ID to allocate if freelist is empty
            uint32_t m_FreeListHead;   // PageID of the head of the freelist
            uint32_t m_RootPageID;     // (optional) root of B+ tree
            uint32_t m_Reserved;       // reserved for future (e.g., transaction ID)

            MetaData()
                : m_NextPageID(1), m_FreeListHead(0), m_RootPageID(0), m_Reserved(0)
            {
            }

            MetaData(uint32_t nextPageID, uint32_t freeListHead)
                : m_NextPageID(nextPageID), m_FreeListHead(freeListHead)
            {
            }
        };

    }
}