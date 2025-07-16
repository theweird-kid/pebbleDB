#include "Page.h"

Page::Page() {
    clear();
}

void Page::setPageID(uint32_t id) {
    header()->m_PageID = id;
}

uint32_t Page::getPageID() const {
    return header()->m_PageID;
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
    header()->m_PageID = 0;
    header()->m_NextPageID = 0;
}

