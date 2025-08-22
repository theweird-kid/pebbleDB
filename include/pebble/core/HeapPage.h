#pragma once

#include "pebble/core/Page.h"

#include <functional>
#include <cstdint>
#include <vector>
#include <string>

namespace pebble {
    namespace core {

        constexpr size_t MAX_SLOTS = 1020;

        class HeapPage
        {
        public:
            explicit HeapPage(Page& page);

            // inserts record -> returns slot number or -1 if no space
            int insert(const std::string& record);

            bool remove(uint16_t slotID);

            std::string get(uint16_t slotID) const;

            void scan(std::function<void(uint16_t, const std::string&)> visitor) const;

        private:
            Page& m_Page;

            struct Slot {
                uint16_t offset;
                uint16_t length;
            };

            uint16_t& numSlots();
            uint16_t numSlots() const;

            Slot getSlot(uint16_t slotID) const;
            void setSlot(uint16_t slotID, const Slot& slot);

            uint16_t freeSpaceOffset() const;
            void setFreeSpaceOffset(uint16_t offset);

            uint16_t headerSize() const;
        };
    }
}