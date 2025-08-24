#pragma once

#include "pebble/core/IFileManager.h"

#include <mutex>
#include <string>
#include <windows.h>

namespace pebble {
    namespace core {

        class WindowsFileManager : public IFileManager {
        public:
            explicit WindowsFileManager(const std::string& filename);
            ~WindowsFileManager();

            void readPage(uint32_t pageID, Page& page) override;
            void writePage(const Page& page) override;

            uint32_t allocatePage() override;
            void freePage(uint32_t pageID) override;

            void flush() override;

            void printFreeList() override;

            bool pageExists(uint32_t pageID) const override;

        private:

            void loadMetaPage();
            void updateMetaPage();

            HANDLE m_FileHandle;        // Windows file handle
            std::string m_Filename;

            uint32_t m_NextPageID = 1;
            uint32_t m_FreeListHead = 0;

            std::recursive_mutex m_RecMutex;

            static constexpr uint32_t META_PAGE_ID = 0;
        };

    }
}