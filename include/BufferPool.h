#pragma once

#include <unordered_map>
#include <list>
#include <cstdint>
#include <stdexcept>

#include "Page.h"
#include "IFileManager.h"

struct Frame {
    Page page;
    bool dirty = false;
    int pinCount = 0;

    Frame() = default;
    Frame(Page&& p) : page(std::move(p)), dirty(false), pinCount(0) {}
};

class BufferPool 
{
public:
    BufferPool(IFileManager& fm, size_t poolSize);

    Page& fetchPage(uint32_t pageID);
    void markDirty(uint32_t pageID);
    void unpinPage(uint32_t pageID);
    void flushPage(uint32_t pageID);
    void flushAll();

private:
    IFileManager& m_FileManager;
    size_t m_MaxPages;

    std::unordered_map<uint32_t, Frame> m_Pages;
    std::list<uint32_t> m_LRU;

    void touchLRU(uint32_t pageID);
    void evictPage();
};
