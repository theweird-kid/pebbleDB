#pragma once

#include "BufferPool.h"
#include "HeapPage.h"

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

class HeapFile
{
public:
    // For loading an existing collection
    HeapFile(const std::string& name, BufferPool& bp, uint32_t startPageID);

    // For creating a new collection
    HeapFile(const std::string& name, BufferPool& bp); // NEW

    uint64_t insert(const std::string& record);
    bool remove(uint64_t recordID);
    std::string get(uint64_t recordID) const;
    void scan(std::function<void(uint64_t, const std::string&)> visitor) const;

    uint32_t getStartPageID() const; // ✅ Needed for catalog manager

private:
    std::string m_Name;
    uint32_t m_StartPageID;
    BufferPool& m_BufferPool;
    std::vector<uint32_t> m_Pages;

    uint64_t makeRecordID(uint32_t pageID, uint16_t slotID) const;
    void parseRecordID(uint64_t recordID, uint32_t& pageID, uint16_t& slotID) const;
};
