#pragma once

#include <unordered_map>
#include <list>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <stdexcept>

#include "Page.h"
#include "IFileManager.h"

struct Frame {
    Page page;
    std::atomic<bool> dirty{ false };                      // modified after loading
    std::atomic<int> pinCount{ 0 };                        // number of clients holding this page

    Frame() = default;
    
    // construct with page move
    explicit Frame(Page&& p)
        : page(std::move(p)), dirty(false), pinCount(0) {}

    // move constructor
    Frame(Frame&& other) noexcept
        : page(std::move(other.page)),
          dirty(other.dirty.load()),
          pinCount(other.pinCount.load())
    {}

    // move assignment
    Frame& operator=(Frame&& other) noexcept {
        if (this != &other) {
            page = std::move(other.page);
            dirty = other.dirty.load();
            pinCount.store(other.pinCount.load());
        }
        return *this;
    }

    // delete copy
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
};

class BufferPool 
{
public:
    BufferPool(IFileManager& fm, size_t poolSize);

	~BufferPool();

    // Fetch page into buffer pool if not already present, pin the page ( pinCount++ )
    Page& fetchPage(uint32_t pageID);

    // Allocate a new Page via FileManager
    uint32_t allocatePage();

    // Mark page as dirty ( modified )
    void markDirty(uint32_t pageID);

    // unpin ( release a pin ) for a page
    void unpinPage(uint32_t pageID);

    // flush a single page to disk
    void flushPage(uint32_t pageID);

    // flush all dirty pages
    void flushAll();

private:
    IFileManager& m_FileManager;
    size_t m_MaxPages;
    std::mutex m_Mutex;

    std::unordered_map<uint32_t, Frame> m_Pages;        // { pageID, Frame }
    std::list<uint32_t> m_LRU;                          // pageIDs, oldest at front

    // update cache
    void touchLRU(uint32_t pageID);

    // evict a victime page
    void evictPage();
};
