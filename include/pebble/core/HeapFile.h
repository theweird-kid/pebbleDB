#pragma once

#include "pebble/core/BufferPool.h"
#include "pebble/core/HeapPage.h"

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace pebble {
    namespace core {

        class HeapFile
        {
        public:
            // For loading an existing collection
            HeapFile(const std::string& name, BufferPool& bp, PageID startPageID);

            // For creating a new collection
            HeapFile(const std::string& name, BufferPool& bp); // NEW

            uint64_t insert(const std::string& record);
            bool remove(uint64_t recordID);
            std::string get(uint64_t recordID) const;
            void scan(std::function<void(uint64_t, const std::string&)> visitor) const;

            PageID getStartPageID() const; // ✅ Needed for catalog manager

        private:
            std::string m_Name;
            PageID m_StartPageID;
            BufferPool& m_BufferPool;
            std::vector<PageID> m_Pages;

            uint64_t makeRecordID(PageID pageID, uint16_t slotID) const;
            void parseRecordID(uint64_t recordID, PageID& pageID, uint16_t& slotID) const;
        };
    }
}
