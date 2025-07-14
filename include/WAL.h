#pragma once

#include <fstream>
#include <string>
#include <cstdint>

class WAL 
{
public:
    WAL(const std::string& logFilename);
    ~WAL();

    void logInsert(uint32_t pageID, const char* data, size_t len);
    void logDelete(uint32_t pageID, const char* data, size_t len);

    void flush();
    void replay();  // redo/undo

private:
    std::ofstream m_LogFile;
};
