#include "HeapPage.h"

#include <iostream>
#include <cstdint>
#include <stdexcept>

HeapPage::HeapPage(Page& page)
    : m_Page(page)
{
    if (m_Page.header()->m_Type == PageType::INVALID) 
    {
        m_Page.header()->m_Type = PageType::HEAP;
        numSlots() = 0;
        setFreeSpaceOffset(PAGE_SIZE);
    }
    else {
        // Still ensure internal fields are valid
        if (freeSpaceOffset() == 0 || freeSpaceOffset() > PAGE_SIZE)
            setFreeSpaceOffset(PAGE_SIZE);

        if (numSlots() > MAX_SLOTS)
            numSlots() = 0; // fallback sanity check
    }
}

uint16_t HeapPage::numSlots() const {
    return *reinterpret_cast<const uint16_t*>(m_Page.data() + HEADER_SIZE);
}

uint16_t& HeapPage::numSlots() {
    return *reinterpret_cast<uint16_t*>(m_Page.data() + HEADER_SIZE);
}

uint16_t HeapPage::freeSpaceOffset() const {
    return *reinterpret_cast<const uint16_t*>(m_Page.data() + HEADER_SIZE + sizeof(uint16_t));
}

void HeapPage::setFreeSpaceOffset(uint16_t offset) {
    *reinterpret_cast<uint16_t*>(m_Page.data() + HEADER_SIZE + sizeof(uint16_t)) = offset;
}

uint16_t HeapPage::headerSize() const {
    return HEADER_SIZE + sizeof(uint16_t) * 2 + numSlots() * sizeof(Slot);
}

HeapPage::Slot HeapPage::getSlot(uint16_t slotID) const 
{
    if(slotID >= numSlots())
        throw std::runtime_error("Invalid slotID");

    const char* base = m_Page.data() + HEADER_SIZE + sizeof(uint16_t) * 2;
    const char* ptr  = base + slotID * sizeof(Slot);
    Slot slot;
    std::memcpy(&slot, ptr, sizeof(Slot));

    return slot;
}

void HeapPage::setSlot(uint16_t slotID, const Slot& slot)
{
    char* base = m_Page.data() + HEADER_SIZE + sizeof(uint16_t) * 2;
    char* ptr  = base + slotID * sizeof(Slot);
    std::memcpy(ptr, &slot, sizeof(Slot));
}

int HeapPage::insert(const std::string& record) {
    uint16_t spaceNeeded = sizeof(Slot) + record.size();
    uint16_t freeSpaceAvailable = freeSpaceOffset() - (headerSize() + sizeof(Slot));

    if (freeSpaceAvailable < record.size()) {
        return -1;  // Not enough space
    }

    // Reuse free slot
    for (uint16_t i = 0; i < numSlots(); ++i) {
        Slot slot = getSlot(i);
        if (slot.length == 0) {
            uint16_t offset = freeSpaceOffset() - record.size();
            if (offset < headerSize()) return -1;
            Slot newSlot{ offset, static_cast<uint16_t>(record.size()) };
            setSlot(i, newSlot);
            std::memcpy(m_Page.data() + offset, record.data(), record.size());
            setFreeSpaceOffset(offset);
            return i;
        }
    }

    // New slot
    uint16_t slotID = numSlots();
    uint16_t offset = freeSpaceOffset() - record.size();
    if (offset < headerSize()) return -1;

    Slot slot{ offset, static_cast<uint16_t>(record.size()) };
    setSlot(slotID, slot);
    std::memcpy(m_Page.data() + offset, record.data(), record.size());
    setFreeSpaceOffset(offset);

    numSlots()++;
    return slotID;
}


bool HeapPage::remove(uint16_t slotID)
{
    Slot slot = getSlot(slotID);
    slot.length = 0;
    setSlot(slotID, slot);
    return true;
}

std::string HeapPage::get(uint16_t slotID) const
{
    Slot slot = getSlot(slotID);
    if(slot.length == 0) return {};

    return std::string(m_Page.data() + slot.offset, slot.length);
}

void HeapPage::scan(std::function<void(uint16_t, const std::string&)> visitor) const
{
    for(uint16_t i = 0; i < numSlots(); i++)
    {
        Slot slot = getSlot(i);
        if(slot.length > 0) {
            visitor(i, std::string(m_Page.data() + slot.offset, slot.length));
        }
    }
}