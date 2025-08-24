#include "pebble/core/CatalogManager.h"
#include <cassert>

using namespace pebble::core;

CatalogManager::CatalogManager(BufferPool& bp)
    : m_BufferPool(bp)
{
    Page& page = m_BufferPool.fetchPage(m_CatalogRootPageID);
    if(page.header()->m_Type != PageType::CATALOG) 
    {
        page.header()->m_Type = PageType::CATALOG;
        page.header()->m_PageID = m_CatalogRootPageID;
        page.header()->m_NextPageID = 0;
        std::memset(page.payload(), 0, PAYLOAD_SIZE);
        m_BufferPool.markDirty(m_CatalogRootPageID);
    }

    m_BufferPool.unpinPage(m_CatalogRootPageID);
    loadCatalog();
}

std::vector<CatalogEntry> CatalogManager::getCollections() const
{
    return m_Entries;
}

CatalogManager::~CatalogManager() {
    if (m_Dirty) {
        saveCatalog();
    }
}


void CatalogManager::createCollection(
    const std::string& name,
    PageID rootPageID,
    PageID heapStartPageID
)
{
    assert(name.size() < 256);

    for(auto& entry: m_Entries) {
        if(entry.name == name)
            return;             // collection already exists
    }

    m_Entries.push_back({ name, rootPageID, heapStartPageID });
    m_Dirty = true;
}

std::optional<std::pair<PageID, PageID>> CatalogManager::getCollectionMeta(
    const std::string& name
)
{
    for (auto& entry : m_Entries) {
        if (entry.name == name) {
            return std::make_pair(entry.rootPageID, entry.heapStartPageID);
        }
    }
    return std::nullopt;
}

void CatalogManager::loadCatalog()
{
    m_Entries.clear();
    PageID current = m_CatalogRootPageID;

    while (current != 0)
    {
        Page& page = m_BufferPool.fetchPage(current);
        auto* header = page.header();
        assert(header->m_Type == PageType::CATALOG);

        const char* ptr = page.payload();
        const char* end = ptr + PAYLOAD_SIZE;

        while (ptr < end && *ptr != 0)
        {
            uint8_t nameLen = static_cast<uint8_t>(*ptr++);
            if (ptr + nameLen + 8 > end)
                break;

            // Collection Name
            std::string name(ptr, nameLen);
            ptr += nameLen;

            // Collection Index page
            PageID indexRootPageID;
            std::memcpy(&indexRootPageID, ptr, sizeof(PageID));
            ptr += sizeof(PageID);

            // Collection Heap page
            PageID heapRootPageID;
            std::memcpy(&heapRootPageID, ptr, sizeof(PageID));
            ptr += sizeof(PageID);

            m_Entries.push_back({ name, indexRootPageID, heapRootPageID });
        }

        current = header->m_NextPageID;
        m_BufferPool.unpinPage(page.getPageID());
    }
}

void CatalogManager::saveCatalog() {
    if (!m_Dirty) return;

    PageID current = m_CatalogRootPageID;
    auto it = m_Entries.begin();

    while (it != m_Entries.end())
    {
        Page& page = m_BufferPool.fetchPage(current);
        auto* header = page.header();
        header->m_Type = PageType::CATALOG;
        header->m_PageID = current;

        char* ptr = page.payload();
        char* end = ptr + PAYLOAD_SIZE;

        while (it != m_Entries.end())
        {
            const CatalogEntry& entry = *it;
            uint8_t nameLen = static_cast<uint8_t>(entry.name.size());
            if (ptr + 1 + nameLen + 8 > end)
                break;

            // Collection Name
            *ptr++ = nameLen;
            std::memcpy(ptr, entry.name.data(), nameLen);
            ptr += nameLen;

            // Collection Index page
            std::memcpy(ptr, &entry.rootPageID, sizeof(PageID));
            ptr += sizeof(PageID);

            // Collection Heap page
            std::memcpy(ptr, &entry.heapStartPageID, sizeof(PageID));
            ptr += sizeof(PageID);

            ++it;
        }

        if (ptr < end) *ptr = 0;   // Terminator
        m_BufferPool.markDirty(current);
        m_BufferPool.unpinPage(current);

        // If more entries remain, allocate next Page
        if (it == m_Entries.end())
        {
            // Done writing entries, Free any extra linked pages ( if exists )
            if (header->m_NextPageID != 0) {
                PageID extra = header->m_NextPageID;
                header->m_NextPageID = 0;
                m_BufferPool.markDirty(current);

                while (extra != 0) {
                    Page& nextPage = m_BufferPool.fetchPage(extra);
                    PageID nxt = nextPage.header()->m_NextPageID;
                    m_BufferPool.freePage(extra);
                    extra = nxt;
                }
            }
            break;
        }

        // Need More space -> Reuse existing | allocate new
        if (header->m_NextPageID != 0) {
            current = header->m_NextPageID;
        }
        else {
            PageID newPage = m_BufferPool.allocatePage();
            Page& newPg = m_BufferPool.fetchPage(newPage);
            newPg.header()->m_Type = PageType::CATALOG;
            newPg.header()->m_PageID = newPage;
            newPg.header()->m_NextPageID = 0;
            std::memset(newPg.payload(), 0, PAYLOAD_SIZE);
            m_BufferPool.markDirty(newPage);
            m_BufferPool.unpinPage(newPage);


            header->m_NextPageID = newPage;
            m_BufferPool.markDirty(page.getPageID());
            current = newPage;
        }
    }

    m_Dirty = false;
}

void CatalogManager::updateCollectionMeta(
    const std::string& name,
    PageID newRootPageID,
    PageID newHeapStartPageID
)
{
    for (auto& entry : m_Entries) {
        if (entry.name == name) {
            entry.rootPageID = newRootPageID;
            entry.heapStartPageID = newHeapStartPageID;
            return;
        }
    }
}
