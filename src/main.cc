#include "../include/WindowsFileManager.h"
#include "../include/Page.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <set>
#include <filesystem>

void test_file_manager() {
    const std::string fname = "testdb.dat";
    std::filesystem::remove(fname);

    std::cout << "🧪 Starting FileManager test...\n";

    {
        WindowsFileManager fm(fname);

        std::vector<uint32_t> pages;

        // allocate 5 pages
        for (int i = 0; i < 5; ++i) {
            uint32_t id = fm.allocatePage();
            std::cout << "Allocated page: " << id << "\n";
            pages.push_back(id);

            Page p;
            p.setPageID(id);
            p.clear();
            p.header()->m_Type = PageType::LEAF;
            p.header()->m_NumKeys = i;
            fm.writePage(p);
        }

        fm.printFreeList();

        // free page 2 and 4
        fm.freePage(pages[1]);
        fm.freePage(pages[3]);

        fm.printFreeList();

        // allocate 2 more pages — these should reuse freed ones
        uint32_t reused1 = fm.allocatePage();
        uint32_t reused2 = fm.allocatePage();

        fm.printFreeList();

        std::cout << "Reused pages: " << reused1 << ", " << reused2 << "\n";

        std::set<uint32_t> freed{pages[1], pages[3]};
        assert(freed.count(reused1) || freed.count(reused2));

        // check that page data can still be written/read
        Page p;
        p.setPageID(reused1);
        p.clear();
        p.header()->m_Type = PageType::INTERNAL;
        p.header()->m_NumKeys = 99;
        fm.writePage(p);

        Page readP;
        fm.readPage(reused1, readP);
        assert(readP.header()->m_Type == PageType::INTERNAL);
        assert(readP.header()->m_NumKeys == 99);
    }

    // reopen and check we can still allocate/free
    {
        WindowsFileManager fm(fname);
        fm.printFreeList();
        uint32_t id = fm.allocatePage();
        std::cout << "After reopen allocated page: " << id << "\n";
        assert(id > 0);
        fm.printFreeList();
    }

    std::filesystem::remove(fname);
    std::cout << "✅ FileManager test passed\n";
}

int main() {
    test_file_manager();
    return 0;
}
