#include "StorageEngine.h"
#include <iostream>

int main() {
    StorageEngine engine("kvstore.db", 10);
    const std::string collectionName = "users";

    if (!engine.createCollection(collectionName)) {
        std::cout << "Collection already exists. Using existing one.\n";
    }
    else {
        std::cout << "Creating New Collection: " << collectionName << std::endl;
    }

    // ---------- RUN 1: Insert ---------
    /*engine.insert(collectionName, 1, "Alice");
    engine.insert(collectionName, 2, "Bob");
    engine.insert(collectionName, 3, "Charlie");
    std::cout << "Inserted 3 users.\n";*/
    

    // ---------- RUN 2: Search ----------
    for (int key : {1, 2, 3}) {
        auto val = engine.get(collectionName, key);
        if (val) {
            std::cout << "Lookup " << key << " => " << *val << "\n";
        }
        else {
            std::cout << "Lookup " << key << " => NOT FOUND\n";
        }
    }

    return 0;
}
