#pragma once

#include <cstdint>
#include <array>
#include <cstring>

constexpr size_t PAGE_SIZE = 4096;

enum class PageType : uint16_t {
    INVALID = 0,
    LEAF = 1,
    INTERNAL = 2,
    META = 3,
    FREE = 4
};

struct PageHeader {
    PageType m_Type;
    uint16_t m_NumKeys;
    uint32_t m_NextPageID;
};

constexpr size_t KEY_SIZE = sizeof(int32_t);
constexpr size_t VALUE_SIZE = sizeof(int32_t);
constexpr size_t PAGEID_SIZE = sizeof(uint32_t);

constexpr size_t ENTRY_SIZE_LEAF = KEY_SIZE + VALUE_SIZE;
constexpr size_t ENTRY_SIZE_INTERNAL = KEY_SIZE + PAGEID_SIZE;

constexpr size_t HEADER_SIZE = sizeof(PageHeader);
constexpr size_t PAYLOAD_SIZE = PAGE_SIZE - HEADER_SIZE;

class Page {
public:
    Page();

    void setPageID(uint32_t id);
    uint32_t getPageID() const;

    // returns only header
    PageHeader* header();
    const PageHeader* header() const;

    // retruns only payload
    char* payload();
    const char* payload() const;

    // returns both header + payload
    char* data();               
    const char* data() const; 

    void clear();

    //
    // Leaf operations
    //
    void setLeafEntry(int idx, int32_t key, int32_t val);
    int32_t getLeafKey(int idx) const;
    int32_t getLeafValue(int idx) const;

    //
    // Internal operations
    //
    void setInternalEntry(int idx, int32_t key, uint32_t rightPtr);
    int32_t getInternalKey(int idx) const;
    uint32_t getInternalChild(int idx) const;

    void setInternalLeftMostChild(uint32_t ptr);
    uint32_t getInternalLeftMostChild() const;

private:
    uint32_t m_PageID = 0;
    std::array<char, PAGE_SIZE> m_Buffer;
};
