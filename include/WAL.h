#pragma once

#include <cstdint>
#include <string>
#include <fstream>

enum class LogType : uint8_t {
    INSERT,
    ERASE,
    BEGIN,
    COMMIT,
    ABORT
};

struct WALRecord {
    LogType type;
    uint32_t txnID;
    uint32_t pageID;
    uint32_t dataLen;
    std::string data;
};

class WAL {
public:
    WAL(const std::string& logFilename);
    ~WAL();

    void logRecord(LogType type, uint32_t txnID, uint32_t pageID, const char* data, size_t len);
    void flush();
    void replay();

private:
    std::ofstream m_LogOut;
    std::ifstream m_LogIn;
    std::string m_LogFilename;

    void writeHeader();
    bool parseRecord(std::istream& in, WALRecord& record);
};
