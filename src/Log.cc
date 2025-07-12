#include "../include/Log.h"

#include <iostream>
#include <cstdio>
#include <string>

#if defined(_WIN32)
#include <windows.h>    // for HANDLE, FlushFileBuffers
#include <io.h>         // for _get_osfhandle
#else
#include <unistd.h>     // for fsync
#endif

#define _CRT_SECURE_NO_WARNINGS     // Disabel Warnings on Windows

void write_to_log(const std::string& filename, const std::string& key, const std::string& value) {
    FILE* file = fopen(filename.c_str(), "a");
    if (!file) {
        throw std::runtime_error("Cannot open file for writing");
    }

    std::string line = key + ":" + value + "\n";
    fwrite(line.data(), 1, line.size(), file);
    fflush(file); // flush to OS buffer

#if defined(_WIN32)
    int fd = _fileno(file);
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    if (!FlushFileBuffers(h)) {
        std::cerr << "Failed to flush to disk on Windows.\n";
    }
#else
    int fd = fileno(file);
    if (fsync(fd) != 0) {
        std::cerr << "Failed to fsync on Linux.\n";
    }
#endif

    fclose(file);
}

void read_log(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        throw std::runtime_error("Cannot open file for reading");
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        std::cout << buffer;
    }

    fclose(file);
}
