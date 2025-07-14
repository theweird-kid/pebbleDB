#pragma once

#include <string>
#include <memory>
#include "Page.h"

class IFileManager {
public:
    virtual ~IFileManager() = default;

    // Read a page from disk into memory.
    virtual void readPage(uint32_t pageID, Page& page) = 0;

    // Write a page from memory to disk.
    virtual void writePage(const Page& page) = 0;

    // Allocate a new page and return its pageID.
    virtual uint32_t allocatePage() = 0;

    // Mark a page as free so it can be reused.
    virtual void freePage(uint32_t pageID) = 0;

    // Flush any buffered writes to disk.
    virtual void flush() = 0;
};
