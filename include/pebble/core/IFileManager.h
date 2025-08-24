#pragma once

#include <string>
#include <memory>
#include "pebble/core/Page.h"

namespace pebble {
    namespace core {

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

            // Check if a page exists in the file.
            virtual bool pageExists(uint32_t pageID) const = 0;

            // Print freelist info (mainly for debugging).
            virtual void printFreeList() = 0;
        };

    } 
} 
