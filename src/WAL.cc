#include "../include/WAL.h"

#include <iostream>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)

#include <windows.h>
#include <io.h>

#else

#include <unistd.h>

#endif

/*  Log Structure

    |   Key Length      |   4 Bytes     |   int32_t  |
    |   Key             |   variable    |   bytes    |
    |   Value Length    |   4 Bytes     |   int32_t  |
    |   Value           |   variable    |   bytes    |

*/


WAL::WAL(const std::string& filename) : m_Filename(filename)
{
    m_File = fopen(filename.c_str(), "ab+");        // Append Binary
    if(!m_File) 
        throw std::runtime_error("Failed to Open WAL File");
}

WAL::~WAL()
{
    if(m_File)
        fclose(m_File);
}

void WAL::flush()
{
    fflush(m_File);

#if defined(_WIN32)
    int fd = _fileno(m_File);
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    FlushFileBuffers(h);
#else
    int fd = fileno(m_File);
    fsync(fd);
#endif
}

void WAL::append(const std::string& key, const std::string& value)
{
    int32_t klen = key.size();
    int32_t vlen = value.size();

    fwrite(&klen, sizeof(klen), 1, m_File);     // Key Length
    fwrite(key.data(), 1, klen, m_File);        // Key
    fwrite(&vlen, sizeof(vlen), 1, m_File);     // Value Length
    fwrite(value.data(), 1, vlen, m_File);       // Value

    flush();
}

void WAL::replay()
{   
    FILE* f = fopen(m_Filename.c_str(), "rb");      // Read Binary
    if(!f) 
        throw std::runtime_error("Failed to open WAL for reading");

    while(true)
    {   
        // Read Key Length
        int32_t klen = 0;
        if(fread(&klen, sizeof(klen), 1, f) != 1) 
            break;

        // Read Value
        std::string key(klen, '\0');
        fread(key.data(), 1, klen, f);
        
        // Read Value Length
        int32_t vlen = 0;
        fread(&vlen, sizeof(vlen), 1, f);
        
        // Read Value
        std::string value(vlen, '\0');
        fread(value.data(), 1, vlen, f);

        std::cout << key << " = " << value << std::endl;

    }

    fclose(f);
}