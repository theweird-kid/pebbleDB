#include "BufferPool.h"
#include "WindowsFileManager.h"
#include <iostream>
#include <vector>

int main() {
    try {
        WindowsFileManager fm("test.db");
        BufferPool bp(fm, 10);

        std::cout << "Allocating 5 pages\n";
        std::vector<uint32_t> ids;

        // Allocate and write to pages
        for (int i = 0; i < 5; ++i) {
            uint32_t id = fm.allocatePage();
            ids.push_back(id);

            Page& page = bp.fetchPage(id);

            page.header()->m_Type = PageType::LEAF;
            page.header()->m_NumKeys = i;
            page.header()->m_NextPageID = 0;

            bp.markDirty(id);
            bp.unpinPage(id);
        }

        // Flush all dirty pages to disk
        std::cout << "Flushing all pages\n";
        bp.flushAll();

        std::cout << "Reading back pages:\n";

        for (uint32_t id : ids) {
            Page& page = bp.fetchPage(id);

            std::cout << "PageID: " << id
                      << ", Type: " << static_cast<int>(page.header()->m_Type)
                      << ", NumKeys: " << page.header()->m_NumKeys
                      << ", NextPageID: " << page.header()->m_NextPageID
                      << "\n";

            bp.unpinPage(id);
        }

        bp.flushAll();

        std::cout << "Freeing pages 2 & 4\n";
        fm.freePage(ids[1]);
        fm.freePage(ids[3]);

        std::cout << "Free Pages\n";
        fm.printFreeList();
        std::cout << "\n";

        // Allocate again — should reuse from freelist
        std::cout << "Allocating 2 more pages (should reuse freed ones):\n";
        for (int i = 0; i < 2; ++i) {
            uint32_t id = fm.allocatePage();
            std::cout << "Allocated PageID: " << id << "\n";
        }

        bp.flushAll();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
