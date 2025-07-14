#include "Page.h"

Page::Page() {
    clear();
}

void Page::setPageID(uint32_t id) {
    m_PageID = id;
}

uint32_t Page::getPageID() const {
    return m_PageID;
}

PageHeader* Page::header() {
    return reinterpret_cast<PageHeader*>(m_Buffer.data());
}

const PageHeader* Page::header() const {
    return reinterpret_cast<const PageHeader*>(m_Buffer.data());
}

char* Page::payload() {
    return m_Buffer.data() + HEADER_SIZE;
}

const char* Page::payload() const {
    return m_Buffer.data() + HEADER_SIZE;
}

char* Page::data() {
    return m_Buffer.data();
}

const char* Page::data() const {
    return m_Buffer.data();
}


void Page::clear() {
    std::memset(m_Buffer.data(), 0, m_Buffer.size());
    header()->m_Type = PageType::INVALID;
    header()->m_NumKeys = 0;
    header()->m_NextPageID = 0;
}

//
// Leaf operations
//
void Page::setLeafEntry(int idx, int32_t key, int32_t val) {
    char* buf = payload() + idx * ENTRY_SIZE_LEAF;
    std::memcpy(buf, &key, KEY_SIZE);
    std::memcpy(buf + KEY_SIZE, &val, VALUE_SIZE);
}

int32_t Page::getLeafKey(int idx) const {
    const char* buf = payload() + idx * ENTRY_SIZE_LEAF;
    int32_t key;
    std::memcpy(&key, buf, KEY_SIZE);
    return key;
}

int32_t Page::getLeafValue(int idx) const {
    const char* buf = payload() + idx * ENTRY_SIZE_LEAF + KEY_SIZE;
    int32_t val;
    std::memcpy(&val, buf, VALUE_SIZE);
    return val;
}

//
// Internal operations
//
void Page::setInternalLeftMostChild(uint32_t ptr) {
    std::memcpy(payload(), &ptr, PAGEID_SIZE);
}

uint32_t Page::getInternalLeftMostChild() const {
    uint32_t ptr;
    std::memcpy(&ptr, payload(), PAGEID_SIZE);
    return ptr;
}

void Page::setInternalEntry(int idx, int32_t key, uint32_t rightPtr) {
    char* buf = payload() + PAGEID_SIZE + idx * ENTRY_SIZE_INTERNAL;
    std::memcpy(buf, &key, KEY_SIZE);
    std::memcpy(buf + KEY_SIZE, &rightPtr, PAGEID_SIZE);
}

int32_t Page::getInternalKey(int idx) const {
    const char* buf = payload() + PAGEID_SIZE + idx * ENTRY_SIZE_INTERNAL;
    int32_t key;
    std::memcpy(&key, buf, KEY_SIZE);
    return key;
}

uint32_t Page::getInternalChild(int idx) const {
    if (idx == 0) {
        return getInternalLeftMostChild();
    }
    const char* buf = payload() + PAGEID_SIZE + (idx - 1) * ENTRY_SIZE_INTERNAL + KEY_SIZE;
    uint32_t ptr;
    std::memcpy(&ptr, buf, PAGEID_SIZE);
    return ptr;
}
