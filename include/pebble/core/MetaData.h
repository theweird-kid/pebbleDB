#pragma once

#include <cstdint>

namespace pebble {
    namespace core {

        struct MetaData {
            PageID m_NextPageID;     // Next page ID to allocate if freelist is empty
            PageID m_FreeListHead;   // PageID of the head of the freelist
            PageID m_CatalogRootPageID;     // (optional) root of B+ tree
            PageID m_Reserved;       // reserved for future (e.g., transaction ID)

            MetaData()
                : m_NextPageID(1), m_FreeListHead(0), m_CatalogRootPageID(1), m_Reserved(0)
            {}

            MetaData(PageID nextPageID, PageID freeListHead)
                : m_NextPageID(nextPageID), m_FreeListHead(freeListHead),
                m_CatalogRootPageID(1), m_Reserved(0)
            {}
        };

    }
}