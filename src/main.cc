#include "CatalogManager.h"
#include "HeapFile.h"
#include "BPlusTree.h"
#include "BufferPool.h"
#include "WindowsFileManager.h"

#include <iostream>
#include <cassert>
#include <optional>
#include <set>

int main() {
    WindowsFileManager fileManager("kvstore.db");
    BufferPool bufferPool(fileManager, 10);

    CatalogManager cm(bufferPool);
    const std::string collectionName = "users";

    std::optional<std::pair<uint32_t, uint32_t>> meta = cm.getCollectionMeta(collectionName);

    BPlusTree* tree;
    HeapFile* heap;

    if (meta.has_value()) {
        std::cout << "Collection found. Loading...\n";
        tree = new BPlusTree(bufferPool, meta->first);
        heap = new HeapFile(collectionName, bufferPool, meta->second);
    }
    else {
        std::cout << "Collection not found. Creating...\n";
        heap = new HeapFile(collectionName, bufferPool);
        tree = new BPlusTree(bufferPool);
        cm.createCollection(collectionName, tree->rootPageID(), heap->getStartPageID());
    }

    // === Insert initial key-value pairs ===
    std::vector<std::pair<int, std::string>> kvs = {
        {10, "Alice"}, {20, "Bob"}, {30, "Charlie"},
        {40, "David"}, {50, "Eva"}, {60, "Frank"},
        {70, "Grace"}, {80, "Hannah"}, {90, "Ian"},
        {100, "Jack"}, {110, "Kathy"}, {120, "Leo"}
    };

    for (auto& [key, value] : kvs) {
        uint64_t rid = heap->insert(value);
        bool split = tree->insert(key, rid);
        if (split) {
            std::cout << "Tree split occurred after inserting key: " << key << "\n";
            cm.updateCollectionMeta(collectionName, tree->rootPageID(), heap->getStartPageID());
        }
        std::cout << "Inserted: " << key << " => " << value << "\n";
    }

    std::cout << "\nInitial Tree:\n";
    tree->print();

    std::cout << "\nInitial Free List:\n";
    fileManager.printFreeList();

    // === Remove keys to cause underflow and trigger page free ===
    std::vector<int> keysToRemove = { 40, 30, 50, 10 };
    for (int key : keysToRemove) {
        std::cout << "\nRemoving key: " << key << "\n";
        bool removed = tree->remove(key);
        assert(removed);

        std::cout << "Tree after deleting " << key << ":\n";
        tree->print();
    }

    std::cout << "\nFree List After Deletion:\n";
    fileManager.printFreeList();

    // === Insert new keys to test page reuse ===
    std::vector<std::pair<int, std::string>> newKVs = {
        {5, "NewOne"}, {15, "NewTwo"}, {35, "NewThree"}, {45, "NewFour"}
    };

    for (auto& [key, value] : newKVs) {
        uint64_t rid = heap->insert(value);
        bool split = tree->insert(key, rid);
        if (split) {
            std::cout << "Tree split occurred after inserting key: " << key << "\n";
            cm.updateCollectionMeta(collectionName, tree->rootPageID(), heap->getStartPageID());
        }
        std::cout << "Inserted: " << key << " => " << value << "\n";
    }

    std::cout << "\nFinal Tree Structure:\n";
    tree->print();

    std::cout << "\nFinal Free List (should have shrunk if reuse happened):\n";
    fileManager.printFreeList();

    // Optional: Print heap contents
    std::cout << "\nHeap Scan:\n";
    heap->scan([](uint64_t rid, const std::string& data) {
        std::cout << "RID " << rid << " -> " << data << "\n";
        });

    std::cout << "\n✅ Page reuse test completed.\n";
}
