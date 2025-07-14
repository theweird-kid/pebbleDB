#include "CatalogManager.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

CatalogManager::CatalogManager(const std::string& catalogFile)
    : m_CatalogFile(catalogFile)
{}

CatalogManager::~CatalogManager()
{
    save();
}

void CatalogManager::load()
{
    std::ifstream in(m_CatalogFile, std::ios::binary);
    if(!in) {
        std::cerr << "Catalog file does not exist. Starting Fresh.\n";
        return;
    }

    uint32_t numTables = 0;
    in.read(reinterpret_cast<char*>(&numTables), sizeof(numTables));

    for(uint32_t idx = 0; idx < numTables; ++idx) {
        TableSchema t = readTable(in);
        m_Tables[t.name] = t;
    }
}

void CatalogManager::save()
{
    std::ofstream out(m_CatalogFile, std::ios::binary | std::ios::trunc);

    uint32_t numTables = static_cast<uint32_t>(m_Tables.size());
    out.write(reinterpret_cast<const char*>(&numTables), sizeof(numTables));

    for(const auto& [name, schema]: m_Tables) {
        writeTable(out, schema);
    }
}

void CatalogManager::createTable(const TableSchema& schema)
{
    if(m_Tables.count(schema.name)) {
        throw std::runtime_error("Table already exists: " + schema.name);
    }
    m_Tables[schema.name] = schema;
}

TableSchema CatalogManager::getTableSchema(const std::string& tableName) const
{
    auto it = m_Tables.find(tableName);
    if(it == m_Tables.end()) {
        throw std::runtime_error("Table not found: " + tableName);
    }
    return it->second;
}

std::vector<std::string> CatalogManager::listTables() const
{
    std::vector<std::string> names;
    for(const auto& [name, _]: m_Tables) {
        names.push_back(name);
    }
    return names;
}

bool CatalogManager::tableExists(const std::string& tableName) const 
{
    return m_Tables.count(tableName) > 0;
}

void CatalogManager::writeTable(std::ofstream& out, const TableSchema& schema) const
{
    uint32_t len = schema.name.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));            // length of schema "name"
    out.write(schema.name.c_str(), len);                                    // schema name

    uint32_t numCols = static_cast<uint32_t>(schema.columns.size());        
    out.write(reinterpret_cast<const char*>(&numCols), sizeof(numCols));    // no. of columns

    for(const auto& col: schema.columns) 
    {
        len = col.name.size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));        // length of col "name"
        out.write(col.name.c_str(), len);                                   // col name

        out.write(reinterpret_cast<const char*>(&col.type), sizeof(col.type));  // col type

        out.write(reinterpret_cast<const char*>(&col.nullable), sizeof(col.nullable));  // col Nullable or not
    }

    len = schema.primaryKey.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));        // length of "primaryKey"
    out.write(schema.primaryKey.c_str(), len);                          // primaryKey

    uint32_t numIdx = static_cast<uint32_t>(schema.indexes.size());     
    out.write(reinterpret_cast<const char*>(&numIdx), sizeof(numIdx));  // no. of indexes

    for(const auto& index: schema.indexes)
    {
        len = index.size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));    // length of index "name"
        out.write(index.c_str(), len);                                  // index name
    }
}

TableSchema CatalogManager::readTable(std::ifstream& in) const 
{
    TableSchema schema;

    uint32_t len = 0;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));        // length of schema "name"
    schema.name.resize(len);                            
    in.read(&schema.name[0], len);                              // schema name

    uint32_t numCols = 0;
    in.read(reinterpret_cast<char*>(&numCols), sizeof(numCols));    // no. of columns

    for(uint32_t idx = 0; idx < numCols; idx++)
    {
        Column col;

        in.read(reinterpret_cast<char*>(&len), sizeof(len));        // length of column "name"
        col.name.resize(len);   
        in.read(&col.name[0], len);                                 // column name

        in.read(reinterpret_cast<char*>(&col.type), sizeof(col.type));          // column type
        in.read(reinterpret_cast<char*>(&col.nullable), sizeof(col.nullable));  // column nullable

        schema.columns.push_back(col);
    }

    in.read(reinterpret_cast<char*>(&len), sizeof(len));                // length of primaryKey
    schema.primaryKey.resize(len);                              
    in.read(&schema.primaryKey[0], len);                                // primaryKey

    uint32_t numIdx = 0;
    in.read(reinterpret_cast<char*>(&numIdx), sizeof(numIdx));          // no. of indexes
    
    for(uint32_t idx = 0; idx < numIdx; idx++)
    {
        std::string index;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));            // length of index "name"
        index.resize(len);
        in.read(&index[0], len);                                        // index

        schema.indexes.push_back(index);
    }

    return schema;
}