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
    HeapFile(const std::string& name, BufferPool& bp);

    uint64_t insert(const std::string& record);
    bool remove(uint64_t recordID);
    std::string get(uint64_t recordID) const;

    void scan(std::function<void(uint64_t, const std::string&)> visitor) const;

private:
    std::string m_Name;
    BufferPool& m_BufferPool;

    std::vector<uint32_t> m_Pages;      // Pages for the heapFile { loaded from catalog }

    uint64_t makeRecordID(uint32_t pageID, uint16_t slotID) const;
    void parseRecordID(uint64_t recordID, uint32_t& pageID, uint16_t& slotID) const;
};