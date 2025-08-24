#include "pebble/core/HeapFile.h"
#include <stdexcept>
#include <iostream>

using namespace pebble::core;

HeapFile::HeapFile(const std::string& name, BufferPool& bp, PageID startPageID)
	: m_Name(name), m_BufferPool(bp), m_StartPageID(startPageID)
{
    if (startPageID == 0 || startPageID == static_cast<PageID>(-1)) {
        throw std::invalid_argument("Invalid startPageID for HeapFile");
    }

    PageID curr = startPageID;
    while (curr != 0 && curr != static_cast<PageID>(-1)) {
        m_Pages.push_back(curr);
        Page& page = m_BufferPool.fetchPage(curr);
        m_BufferPool.unpinPage(curr);
        curr = page.header()->m_NextPageID;
    }
}

// ✅ Constructor for new collection creation
HeapFile::HeapFile(const std::string& name, BufferPool& bp)
    : m_Name(name), m_BufferPool(bp)
{
    PageID pageID = m_BufferPool.allocatePage();
    Page& page = m_BufferPool.fetchPage(pageID);

	m_StartPageID = pageID;

    page.header()->m_Type = PageType::HEAP;
    page.header()->m_PageID = pageID;
    page.header()->m_NextPageID = 0;

    m_BufferPool.markDirty(pageID);
    m_BufferPool.unpinPage(pageID);

    m_Pages.push_back(pageID);
}

uint32_t HeapFile::getStartPageID() const {
    return m_StartPageID;
}

uint64_t HeapFile::makeRecordID(uint32_t pageID, uint16_t slotID) const {
    return (static_cast<uint64_t>(pageID) << 16) | slotID;
}

void HeapFile::parseRecordID(uint64_t recordID, uint32_t& pageID, uint16_t& slotID) const {
    pageID = static_cast<PageID>(recordID >> 16);
    slotID = static_cast<uint16_t>(recordID & 0xFFFF);
}

uint64_t HeapFile::insert(const std::string& record) {
    for (PageID pageID : m_Pages) {
        Page& p = m_BufferPool.fetchPage(pageID);
        HeapPage hp(p);
        int slotID = hp.insert(record);
        if (slotID >= 0) {
            m_BufferPool.markDirty(pageID);
            m_BufferPool.unpinPage(pageID);
            return makeRecordID(pageID, slotID);
        }
        m_BufferPool.unpinPage(pageID);
    }

    PageID newPageID = m_BufferPool.allocatePage();
    Page& newPage = m_BufferPool.fetchPage(newPageID);

    if (!m_Pages.empty()) {
        PageID lastPageID = m_Pages.back();
        Page& lastPage = m_BufferPool.fetchPage(lastPageID);
        lastPage.header()->m_NextPageID = newPageID;
        m_BufferPool.markDirty(lastPageID);
        m_BufferPool.unpinPage(lastPageID);
    }

    newPage.header()->m_Type = PageType::HEAP;
    newPage.header()->m_PageID = newPageID;
    newPage.header()->m_NextPageID = 0;

    HeapPage hp(newPage);
    int slotID = hp.insert(record);

    m_BufferPool.markDirty(newPageID);
    m_BufferPool.unpinPage(newPageID);

    m_Pages.push_back(newPageID);
    return makeRecordID(newPageID, slotID);
}

bool HeapFile::remove(uint64_t recordID) {
    PageID pageID;
    uint16_t slotID;
    parseRecordID(recordID, pageID, slotID);

    Page& page = m_BufferPool.fetchPage(pageID);
    HeapPage hp(page);

    bool ok = hp.remove(slotID);
    if (ok)
        m_BufferPool.markDirty(pageID);

    m_BufferPool.unpinPage(pageID);
    return ok;
}

std::string HeapFile::get(uint64_t recordID) const {
    PageID pageID;
    uint16_t slotID;
    parseRecordID(recordID, pageID, slotID);

    Page& page = m_BufferPool.fetchPage(pageID);
    HeapPage hp(page);
    std::string data = hp.get(slotID);

    m_BufferPool.unpinPage(pageID);
    return data;
}

void HeapFile::scan(std::function<void(uint64_t, const std::string&)> visitor) const {
    for (PageID pageID : m_Pages) {
        Page& page = m_BufferPool.fetchPage(pageID);
        HeapPage hp(page);
        hp.scan([&](uint16_t slotID, const std::string& record) {
            visitor(makeRecordID(pageID, slotID), record);
        });
        m_BufferPool.unpinPage(pageID);
    }
}
