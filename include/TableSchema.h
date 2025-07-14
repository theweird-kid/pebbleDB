#pragma once

#include <string>
#include <vector>

enum class ColumnType : uint8_t {
    INT,
    TEXT,
    FLOAT
};

struct Column {
    std::string name;
    ColumnType type;
    bool nullable = true;
};

struct TableSchema {
    std::string name;
    std::vector<Column> columns;
    std::string primaryKey;
    std::vector<std::string> indexes;
};