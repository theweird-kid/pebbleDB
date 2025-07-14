#pragma once

#include "TableSchema.h"

#include <unordered_map>

class CatalogManager
{
public:
    CatalogManager(const std::string& catalogFile);
    ~CatalogManager();

    void load();
    void save();

    void createTable(const TableSchema& schema);
    TableSchema getTableSchema(const std::string& tableName) const;
    std::vector<std::string> listTables() const;

    bool tableExists(const std::string& tableName) const;

private:
    std::string m_CatalogFile;
    std::unordered_map<std::string, TableSchema> m_Tables;

    void writeTable(std::ofstream& out, const TableSchema& schema) const;
    TableSchema readTable(std::ifstream& in) const;
};