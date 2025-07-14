#include "../include/WAL.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>  // C++17

WAL::WAL(const std::string& logFilename)
    : m_LogFilename(logFilename)
{
    bool exists = std::filesystem::exists(m_LogFilename);

    m_LogOut.open(m_LogFilename, std::ios::binary | std::ios::app);
    if (!m_LogOut) {
        throw std::runtime_error("Failed to open WAL file for writing: " + m_LogFilename);
    }

    if (!exists) {
        writeHeader();
    }
}

WAL::~WAL()
{
    flush();
    if (m_LogOut.is_open())
        m_LogOut.close();
}

void WAL::writeHeader()
{
    static const char magic[] = "WALv1";
    m_LogOut.write(magic, sizeof(magic));
}

void WAL::logRecord(LogType type, uint32_t txnID, uint32_t pageID, const char* data, size_t len)
{
    m_LogOut.write(reinterpret_cast<const char*>(&type), sizeof(type));
    m_LogOut.write(reinterpret_cast<const char*>(&txnID), sizeof(txnID));
    m_LogOut.write(reinterpret_cast<const char*>(&pageID), sizeof(pageID));

    uint32_t dataLen = static_cast<uint32_t>(len);
    m_LogOut.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));

    if (len > 0) {
        m_LogOut.write(data, len);
    }
}

void WAL::flush()
{
    m_LogOut.flush();
}

bool WAL::parseRecord(std::istream& in, WALRecord& record)
{
    if (!in.read(reinterpret_cast<char*>(&record.type), sizeof(record.type))) return false;
    if (!in.read(reinterpret_cast<char*>(&record.txnID), sizeof(record.txnID))) return false;
    if (!in.read(reinterpret_cast<char*>(&record.pageID), sizeof(record.pageID))) return false;
    if (!in.read(reinterpret_cast<char*>(&record.dataLen), sizeof(record.dataLen))) return false;

    record.data.resize(record.dataLen);
    if (!in.read(&record.data[0], record.dataLen)) return false;

    return true;
}

void WAL::replay()
{
    std::cout << "Replaying WAL...\n";

    flush();

    m_LogIn.open(m_LogFilename, std::ios::binary);
    if (!m_LogIn) {
        throw std::runtime_error("Failed to open WAL file for reading: " + m_LogFilename);
    }

    // skip header
    char header[6];
    if (!m_LogIn.read(header, sizeof(header))) {
        throw std::runtime_error("WAL file is empty or corrupt (missing header)");
    }

    while (true) {
        WALRecord record;
        if (!parseRecord(m_LogIn, record)) break;

        std::cout << "WAL Record - TxnID: " << record.txnID << ", ";

        switch (record.type) {
            case LogType::BEGIN:   std::cout << "BEGIN"; break;
            case LogType::COMMIT:  std::cout << "COMMIT"; break;
            case LogType::ABORT:   std::cout << "ABORT"; break;
            case LogType::INSERT:  std::cout << "INSERT"; break;
            case LogType::ERASE:   std::cout << "ERASE"; break;
            default:               std::cout << "UNKNOWN"; break;
        }

        std::cout << ", PageID: " << record.pageID
                  << ", DataLen: " << record.dataLen << "\n";
    }

    m_LogIn.close();
}
