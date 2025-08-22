#pragma once

#include "pebble/core/IFileManager.h"
#include <string>

namespace pebble {
    namespace core {

        class PosixFileManager : public IFileManager {
        public:
            PosixFileManager(const std::string& filename);
            ~PosixFileManager();

            void readPage(uint32_t pageID, Page& page) override;
            void writePage(const Page& page) override;
            uint32_t allocatePage() override;
            void freePage(uint32_t pageID) override;
            void flush() override;

        private:
            int m_fd;               // POSIX file descriptor
            uint32_t m_nextPageID;
        };

    }
}