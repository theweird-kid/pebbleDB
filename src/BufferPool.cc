#include "BufferPool.h"
#include <iostream>

BufferPool::BufferPool(IFileManager& fm, size_t poolSize)
    : m_FileManager(fm), m_MaxPages(poolSize)
{}

BufferPool::~BufferPool()
{
    flushAll();  // Ensure all dirty pages are flushed to disk
}

Page& BufferPool::fetchPage(uint32_t pageID)
{   
    std::lock_guard<std::mutex> lock(m_Mutex);

	// META PAGE
    if (pageID == 0) {
        if(!m_FileManager.pageExists(pageID)) {
			allocatePage();
		}
    }
    
	// CATALOG PAGE
    if(pageID == 1) {
        if(!m_FileManager.pageExists(pageID)) {
			allocatePage();  // Ensure Catalog page exists
		}
	}

    auto it = m_Pages.find(pageID);
    if(it != m_Pages.end())         // Page found in memory
    {
        it->second.pinCount++;
        touchLRU(pageID);
        return it->second.page;
    }

    if(m_Pages.size() >= m_MaxPages) {
        evictPage();
    }

    // Load page from disk
    Page p;
    m_FileManager.readPage(pageID, p);

    // pin the page
    Frame frame(std::move(p));
    frame.pinCount = 1;

    // cache the page in memory
    m_Pages[pageID] = std::move(frame);
    m_LRU.push_front(pageID);

    return m_Pages[pageID].page;
}

uint32_t BufferPool::allocatePage()
{
    return m_FileManager.allocatePage();
}

void BufferPool::markDirty(uint32_t pageID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Pages.find(pageID);
    if(it != m_Pages.end()) {
        it->second.dirty = true;
    }
}

void BufferPool::unpinPage(uint32_t pageID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Pages.find(pageID);
    if(it != m_Pages.end()) {
        it->second.pinCount -= 1;
    }
}

void BufferPool::flushPage(uint32_t pageID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Pages.find(pageID);
    if(it != m_Pages.end() && it->second.dirty) {
        std::cout << "flushPage(" << pageID << ")\n";
        m_FileManager.writePage(it->second.page);
        it->second.dirty = false;
    }
}

void BufferPool::flushAll()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    for(auto &[pageID, frame]: m_Pages) {
        if(frame.dirty) {
            m_FileManager.writePage(frame.page);
            frame.dirty = false;
        }
    }
}

void BufferPool::touchLRU(uint32_t pageID)
{
    m_LRU.remove(pageID);
    m_LRU.push_front(pageID);
}

void BufferPool::evictPage()
{
    for(auto it = m_LRU.rbegin(); it != m_LRU.rend(); it++) 
    {
        uint32_t victimID = *it;
        auto& frame = m_Pages[victimID];

        if(frame.pinCount == 0) 
        {
            if(frame.dirty) {
                m_FileManager.writePage(frame.page);
            }
            m_Pages.erase(victimID);
            m_LRU.erase(std::next(it).base());
            return;
        }
    }

    throw std::runtime_error("No unpinned pages available for eviction");
}

void BufferPool::freePage(uint32_t pageID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Remove from buffer pool if present
    m_Pages.erase(pageID);
    m_LRU.remove(pageID);

    // Free from disk and update freelist
    m_FileManager.freePage(pageID);
}
