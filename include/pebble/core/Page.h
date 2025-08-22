#pragma once

#include <cstdint>
#include <array>
#include <cstring>

namespace pebble {
    namespace core
    {

        constexpr size_t PAGE_SIZE = 4096;

        using PageID = uint32_t;

        enum class PageType : uint32_t {
            INVALID = 0,
            LEAF = 1,
            INTERNAL = 2,
            META = 3,
            FREE = 4,
            HEAP = 5
        };

        struct PageHeader {
            PageType m_Type;
            uint32_t m_PageID;
            uint32_t m_NextPageID;
        };


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

        private:
            std::array<char, PAGE_SIZE> m_Buffer;
        };

    }
}