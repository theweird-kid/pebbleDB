#pragma once

#include "pebble/core/Page.h"
#include "pebble/core/BufferPool.h"

#include <string>
#include <vector>
#include <optional>

namespace pebble
{
    namespace core {

        struct CatalogEntry {
            std::string name;
            PageID rootPageID;
            PageID heapStartPageID;
        };

        class CatalogManager {
        public:
            CatalogManager(BufferPool& bufferPool);

            void createCollection(const std::string& name, PageID rootPageID, PageID heapStartPageID);

            std::optional<std::pair<PageID, PageID>> getCollectionMeta(const std::string& name);

            void updateCollectionMeta(const std::string& name, PageID newRootPageID, PageID newHeapStartPageID);

        private:
            BufferPool& m_BufferPool;
            const PageID m_CatalogPageID = 1;

            std::vector<CatalogEntry> loadCatalog() const;
            void saveCatalog(const std::vector<CatalogEntry>& entries);
        };

    }
}
