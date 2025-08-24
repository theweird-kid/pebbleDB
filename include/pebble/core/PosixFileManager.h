#pragma once

#include "pebble/core/IFileManager.h"

#include <string>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <stdexcept>

namespace pebble {
    namespace core {

        class PosixFileManager : public IFileManager {
        public:
            explicit PosixFileManager(const std::string& filename);
            ~PosixFileManager();

            void readPage(uint32_t pageID, Page& page) override;
            void writePage(const Page& page) override;

            uint32_t allocatePage() override;
            void freePage(uint32_t pageID) override;

            void flush() override;

            bool pageExists(uint32_t pageID) const override;
            void printFreeList() override;

        private:
            void loadMetaPage();
            void updateMetaPage();

            int m_Fd;                  // POSIX file descriptor
            std::string m_Filename;

            uint32_t m_NextPageID = 1;  // from meta page
            uint32_t m_FreeListHead = 0;

            mutable std::recursive_mutex m_RecMutex;

            static constexpr uint32_t META_PAGE_ID = 0;
        };

    } 
} 
