#include "CatalogManager.h"
#include <cassert>

CatalogManager::CatalogManager(BufferPool& bp)
    : m_BufferPool(bp)
{
    Page& page = m_BufferPool.fetchPage(m_CatalogPageID);
    if(page.header()->m_Type != PageType::META) {
        page.header()->m_Type = PageType::META;
        page.header()->m_PageID = m_CatalogPageID;
        page.header()->m_NextPageID = 0;
        std::memset(page.payload(), 0, PAYLOAD_SIZE);
        m_BufferPool.markDirty(m_CatalogPageID);
    }

    m_BufferPool.unpinPage(m_CatalogPageID);
}

void CatalogManager::createCollection(
    const std::string& name,
    uint32_t rootPageID,
    uint32_t heapStartPageID
)
{
    assert(name.size() < 256);

    auto entries = loadCatalog();
    for(auto& entry: entries) {
        if(entry.name == name)
            return;             // collection already exists
    }

    entries.push_back({ name, rootPageID, heapStartPageID });
    saveCatalog(entries);
}

std::optional<std::pair<uint32_t, uint32_t>> CatalogManager::getCollectionMeta(
    const std::string& name
)
{
    auto entries = loadCatalog();
    for (auto& entry : entries) {
        if (entry.name == name) {
            return std::make_pair(entry.rootPageID, entry.heapStartPageID);
        }
    }
    return std::nullopt;
}

std::vector<CatalogEntry> CatalogManager::loadCatalog() const
{
    Page& page = m_BufferPool.fetchPage(m_CatalogPageID);
    const char* ptr = page.payload();
    const char* end = ptr + PAYLOAD_SIZE;

    std::vector<CatalogEntry> entries;
    while(ptr < end && (*ptr) != 0)
    {
        uint8_t nameLen = static_cast<uint8_t>(*ptr++);
        if(ptr + nameLen + 8 > end) break;

        std::string name(ptr, nameLen);
        ptr += nameLen;

        uint32_t rootPageID;
        std::memcpy(&rootPageID, ptr, 4);
        ptr += 4;

        uint32_t heapStartPageID;
        std::memcpy(&heapStartPageID, ptr, 4);
        ptr += 4;

        entries.push_back({ name, rootPageID, heapStartPageID });
    }

    m_BufferPool.unpinPage(m_CatalogPageID);
    return entries;
}

void CatalogManager::saveCatalog(const std::vector<CatalogEntry>& entries) {
    Page& page = m_BufferPool.fetchPage(m_CatalogPageID);
    char* ptr = page.payload();
    char* end = ptr + PAYLOAD_SIZE;

    for (const auto& entry : entries) {
        uint8_t nameLen = static_cast<uint8_t>(entry.name.size());
        if (ptr + 1 + nameLen + 8 > end) break;

        *ptr++ = nameLen;
        std::memcpy(ptr, entry.name.data(), nameLen);
        ptr += nameLen;

        std::memcpy(ptr, &entry.rootPageID, 4);
        ptr += 4;

        std::memcpy(ptr, &entry.heapStartPageID, 4);
        ptr += 4;
    }

    // Zero out the rest of the payload to mark the end
    if (ptr < end) {
        *ptr = 0;
    }

    m_BufferPool.markDirty(m_CatalogPageID);
    m_BufferPool.unpinPage(m_CatalogPageID);
}

void CatalogManager::updateCollectionMeta(
    const std::string& name,
    uint32_t newRootPageID,
    uint32_t newHeapStartPageID
)
{
    auto entries = loadCatalog();
    for (auto& entry : entries) {
        if (entry.name == name) {
            entry.rootPageID = newRootPageID;
            entry.heapStartPageID = newHeapStartPageID;
            saveCatalog(entries);
            return;
        }
    }
}
