#include "pebble/core/CatalogManager.h"
#include <cassert>

using namespace pebble::core;

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
    PageID rootPageID,
    PageID heapStartPageID
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

std::optional<std::pair<PageID, PageID>> CatalogManager::getCollectionMeta(
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

        PageID rootPageID;
        std::memcpy(&rootPageID, ptr, sizeof(PageID));
        ptr += sizeof(PageID);

        PageID heapStartPageID;
        std::memcpy(&heapStartPageID, ptr, sizeof(PageID));
        ptr += sizeof(PageID);

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

        std::memcpy(ptr, &entry.rootPageID, sizeof(PageID));
        ptr += sizeof(PageID);

        std::memcpy(ptr, &entry.heapStartPageID, sizeof(PageID));
        ptr += sizeof(PageID);
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
    PageID newRootPageID,
    PageID newHeapStartPageID
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
