#pragma once

#include <string>

// Writes a key-value pair to the log file.
// Ensures the write is flushed and durable.
void write_to_log(const std::string& filename, const std::string& key, const std::string& value);

// Reads and prints the contents of the log file to stdout.
void read_log(const std::string& filename);
