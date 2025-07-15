#include "HeapFile.h"

#include <stdexcept>
#include <iostream>

HeapFile::HeapFile(const std::string& name, BufferPool& bp)
    : m_Name(name), m_BufferPool(bp)
{
    
}

uint64_t HeapFile::makeRecordID(uint32_t pageID, uint16_t slotID) const
{
    return (static_cast<uint64_t>(pageID) << 16) | slotID;
}

void HeapFile::parseRecordID(uint64_t recordID, uint32_t& pageID, uint16_t& slotID) const 
{
    pageID = static_cast<uint32_t>(recordID >> 16);
    slotID = static_cast<uint16_t>(recordID & 0xFFFF);
}

uint64_t HeapFile::insert(const std::string& record)
{
    // Check within currently allocated Pages for space
    for(uint32_t pageID: m_Pages)
    {
        Page& p = m_BufferPool.fetchPage(pageID);
        HeapPage hp(p);
        int slotID = hp.insert(record);
        std::cout << "[DEBUG] SlotID: " << slotID << std::endl;
        if(slotID >= 0) {
            m_BufferPool.markDirty(pageID);
            m_BufferPool.unpinPage(pageID);
            std::cout << "[DEBUG]:" << std::string(p.data(), PAGE_SIZE) << std::endl;
            return makeRecordID(pageID, slotID);
        }
        m_BufferPool.unpinPage(pageID);
    }

    // Allocate a New Page
    uint32_t newPageID = m_BufferPool.allocatePage();
    m_Pages.push_back(newPageID);

    // Fetch newly allocated page
    Page& p = m_BufferPool.fetchPage(newPageID);
    HeapPage hp(p);
    int slotID = hp.insert(record);

    m_BufferPool.markDirty(newPageID);
    m_BufferPool.unpinPage(newPageID);

    std::cout << "[DEBUG] m_Pages: " << m_Pages.size() << std::endl;
    std::cout << "[DEBUG]:" << std::string(p.data(), PAGE_SIZE) << std::endl;

    return makeRecordID(newPageID, slotID);
}

bool HeapFile::remove(uint64_t recordID)
{
    uint32_t pageID;
    uint16_t slotID;
    parseRecordID(recordID, pageID, slotID);

    Page& page = m_BufferPool.fetchPage(pageID);
    HeapPage hp(page);

    bool ok = hp.remove(slotID);
    if(ok) 
        m_BufferPool.markDirty(pageID);

    m_BufferPool.unpinPage(pageID);
    return ok;
}

std::string HeapFile::get(uint64_t recordID) const
{
    uint32_t pageID;
    uint16_t slotID;
    parseRecordID(recordID, pageID, slotID);

    Page& page = m_BufferPool.fetchPage(pageID);
    HeapPage hp(page);
    std::string data = hp.get(slotID);

    m_BufferPool.unpinPage(pageID);
    return data;
}

void HeapFile::scan(std::function<void(uint64_t, const std::string&)> visitor) const
{
    for(uint32_t pageID: m_Pages) 
    {
        Page& page = m_BufferPool.fetchPage(pageID);
        HeapPage hp(page);

        hp.scan([&](uint16_t slotID, const std::string& record) {
            visitor(makeRecordID(pageID, slotID), record);
        });

        m_BufferPool.unpinPage(pageID);
    }
}