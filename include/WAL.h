#pragma once

#include <string>

// Write Ahead Logging
/*  Log Structure

    |   Key Length      |   4 Bytes     |   int32_t  |
    |   Key             |   variable    |   bytes    |
    |   Value Length    |   4 Bytes     |   int32_t  |
    |   Value           |   variable    |   bytes    |

*/


class WAL
{
public:
    WAL(const std::string& filename);
    ~WAL();

    // Write Log
    void append(const std::string& key, const std::string& value);

    // Replay Log
    void replay();

private:
    // File Discriptor for the currently opened File
    FILE* m_File;       
    // File name for the current file
    std::string m_Filename;

    // Persist updates to Storage
    void flush();
};