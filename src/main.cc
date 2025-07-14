#include "CatalogManager.h"
#include <iostream>

int main() {
    CatalogManager catalog("catalog.db");
    catalog.load();

    if (!catalog.tableExists("users")) {
        TableSchema users;
        users.name = "users";
        users.primaryKey = "id";
        users.columns = {
            {"id", ColumnType::INT, false},
            {"name", ColumnType::TEXT, true},
            {"age", ColumnType::INT, true}
        };

        catalog.createTable(users);
    }

    for (const auto& tbl : catalog.listTables()) {
        auto schema = catalog.getTableSchema(tbl);
        std::cout << "Table: " << schema.name << "\n";
        for (const auto& col : schema.columns) {
            std::cout << "   Column: " << col.name 
                      << " Type: " << static_cast<int>(col.type) 
                      << " Nullable: " << col.nullable << "\n";
        }
        std::cout << "   PrimaryKey: " << schema.primaryKey << "\n";
        for(const auto& idx: schema.indexes)
        {
            std::cout << "  Index: " << idx << "\n";
        }
        std::cout << "\n";
    }

    catalog.save();

    return 0;
}
