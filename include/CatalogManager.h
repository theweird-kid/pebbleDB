#pragma once

#include "Page.h"
#include "BufferPool.h"

#include <string>
#include <vector>
#include <optional>

struct CatalogEntry {
    std::string name;
    uint32_t rootPageID;
    uint32_t heapStartPageID;
};

class CatalogManager {
public:
    CatalogManager(BufferPool& bufferPool);

    void createCollection(const std::string& name, uint32_t rootPageID, uint32_t heapStartPageID);

    std::optional<std::pair<uint32_t, uint32_t>> getCollectionMeta(const std::string& name);

    void updateCollectionMeta(const std::string& name, uint32_t newRootPageID, uint32_t newHeapStartPageID);

private:
    BufferPool& m_BufferPool;
    const uint32_t m_CatalogPageID = 1;

    std::vector<CatalogEntry> loadCatalog() const;
    void saveCatalog(const std::vector<CatalogEntry>& entries);
};
