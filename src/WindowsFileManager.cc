#include "WindowsFileManager.h"
#include "MetaData.h"

#include <iostream>
#include <stdexcept>
#include <cstring>

WindowsFileManager::WindowsFileManager(const std::string& filename)
    : m_Filename(filename)
{
    m_FileHandle = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(m_FileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // TODO: read metadata from file if exists to get m_NextPageID
    loadMetaPage();
}

WindowsFileManager::~WindowsFileManager() 
{
    if(m_FileHandle != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(m_FileHandle);
        CloseHandle(m_FileHandle);
    }
}

void WindowsFileManager::loadMetaPage()
{
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    MetaData meta {};
    DWORD bytesRead = 0;

    SetFilePointer(m_FileHandle, 0, nullptr, FILE_BEGIN);

    if(!ReadFile(m_FileHandle, &meta, sizeof(meta), &bytesRead, nullptr) || bytesRead != sizeof(MetaData)) 
    {
        // File is empty, initialize meta page
        this->m_NextPageID = 1;
        this->m_FreeListHead = 0;
        updateMetaPage();
    }
    else
    {
        this->m_NextPageID = meta.m_NextPageID;
        this->m_FreeListHead = meta.m_FreeListHead;
    }
}

void WindowsFileManager::updateMetaPage() 
{
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    MetaData meta { m_NextPageID, m_FreeListHead };
    SetFilePointer(m_FileHandle, 0, nullptr, FILE_BEGIN);
    DWORD written = 0;
    if (!WriteFile(m_FileHandle, &meta, sizeof(MetaData), &written, nullptr) || written != sizeof(MetaData)) {
        throw std::runtime_error("Failed to write MetaData page");
    }
}

void WindowsFileManager::readPage(uint32_t pageID, Page& page) {
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(pageID) * PAGE_SIZE;

    DWORD low = static_cast<DWORD>(offset.QuadPart);
    LONG high = static_cast<LONG>(offset.QuadPart >> 32);

    DWORD pos = SetFilePointer(m_FileHandle, low, &high, FILE_BEGIN);
    if (pos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        throw std::runtime_error("Failed to seek to page " + std::to_string(pageID));
    }

    DWORD bytesRead = 0;
    BOOL result = ReadFile(m_FileHandle, page.data(), PAGE_SIZE, &bytesRead, nullptr);
    if (!result || bytesRead != PAGE_SIZE) {
        throw std::runtime_error("Failed to read page " + std::to_string(pageID) +
                                 " (read " + std::to_string(bytesRead) + " bytes)");
    }

    page.setPageID(pageID);
}


void WindowsFileManager::writePage(const Page& page) {
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(page.getPageID()) * PAGE_SIZE;

    DWORD low = static_cast<DWORD>(offset.QuadPart);
    LONG high = static_cast<LONG>(offset.QuadPart >> 32);

    DWORD pos = SetFilePointer(m_FileHandle, low, &high, FILE_BEGIN);
    if (pos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        throw std::runtime_error("Failed to seek to page " + std::to_string(page.getPageID()));
    }

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(m_FileHandle, page.data(), PAGE_SIZE, &bytesWritten, nullptr);
    if (!result || bytesWritten != PAGE_SIZE) {
        throw std::runtime_error("Failed to write page " + std::to_string(page.getPageID()));
    }

    // Ensure data is flushed to disk
    FlushFileBuffers(m_FileHandle);
}


uint32_t WindowsFileManager::allocatePage()
{
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    uint32_t pageID;
    if (m_FreeListHead != 0)
    {
        // reuse page from freelist
        pageID = m_FreeListHead;

        std::cout << "[PAGE Reused]: " << m_FreeListHead << std::endl;

        Page page;
        readPage(pageID, page);
        m_FreeListHead = page.header()->m_NextPageID;
    }
    else
    {
        std::cout << "[PAGE NEW]: " << m_NextPageID++ << std::endl;
        pageID = m_NextPageID++;
    }

    // Create a blank page and write it to disk
    Page blank;
    blank.setPageID(pageID);
    writePage(blank);

    updateMetaPage();
    return pageID;
}


void WindowsFileManager::freePage(uint32_t pageID)
{
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);

    Page page;
    page.setPageID(pageID);
    page.clear();

    page.header()->m_Type = PageType::FREE;
    page.header()->m_NextPageID = m_FreeListHead;

    writePage(page);

    m_FreeListHead = pageID;
    updateMetaPage();
}

void WindowsFileManager::flush() 
{
    std::lock_guard<std::recursive_mutex> lock(m_RecMutex);    

    if (!FlushFileBuffers(m_FileHandle)) {
        throw std::runtime_error("Failed to flush file buffers");
    }
}

void WindowsFileManager::printFreeList() {
    uint32_t current = m_FreeListHead;
    while (current != 0) {
        Page page;
        readPage(current, page);
        std::cout << "Free Page: " << current << "\n";
        current = page.header()->m_NextPageID;
    }
}

bool WindowsFileManager::pageExists(uint32_t pageID) const {
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(m_FileHandle, &fileSize)) {
        throw std::runtime_error("Failed to get file size");
    }

    LONGLONG pageOffset = static_cast<LONGLONG>(pageID) * PAGE_SIZE;
    return pageOffset + PAGE_SIZE <= fileSize.QuadPart;
}
