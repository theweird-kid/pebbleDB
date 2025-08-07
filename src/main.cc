#include "CatalogManager.h"
#include "HeapFile.h"
#include "BPlusTree.h"
#include "BufferPool.h"
#include "WindowsFileManager.h"

#include <iostream>
#include <cassert>
#include <optional>

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
    } else {
        std::cout << "Collection not found. Creating...\n";
        heap = new HeapFile(collectionName, bufferPool);
        tree = new BPlusTree(bufferPool);
        cm.createCollection(collectionName, tree->rootPageID(), heap->getStartPageID());
    }

    // Insert key-value pairs
    std::vector<std::pair<int, std::string>> kvs = {
        {10, "Alice"}, {20, "Bob"}, {30, "Charlie"},
        {40, "David"}, {50, "Eva"}, {60, "Frank"},
        {70, "Grace"}, /*{80, "Hannah"}, {90, "Ian"},
		{100, "Jack"}, {110, "Kathy"}, {120, "Leo"}*/
    };
    // === Insert key-value pairs ===
    for (auto& [key, value] : kvs) {
        uint64_t rid = heap->insert(value);
        bool split = tree->insert(key, rid);
        if (split) {
            std::cout << "Tree split occurred after inserting key: " << key << "\n";
            cm.updateCollectionMeta(collectionName,
                tree->rootPageID(),
                heap->getStartPageID()
            );
        }
        std::cout << "Inserted: " << key << " => " << value << "\n";
    }

    std::cout << "\nVerifying lookups:\n";
    for (auto& [key, expectedValue] : kvs) {
        auto ridOpt = tree->search(key);
        assert(ridOpt.has_value());
        std::string actual = heap->get(ridOpt.value());
        std::cout << key << " -> " << actual << "\n";
        assert(actual == expectedValue);
    }

    tree->print();

    // === Test remove ===
    std::vector<int> keysToRemove = {40, 30, 50, 10 }; // Pick keys from different leaf nodes
    for (int key : keysToRemove) {
        std::cout << "\nRemoving key: " << key << "\n";
        bool removed = tree->remove(key);
        assert(removed);

        std::cout << "\nTree after deleting " << key << ":\n";
        tree->print();

        auto ridOpt = tree->search(key);
        std::cout << "Searching for " << key << ": ";
        assert(!ridOpt.has_value());
        std::cout << "not found ✅\n";
    }

    // === Re-verify remaining keys ===
    std::cout << "\nVerifying remaining keys:\n";
    for (auto& [key, expectedValue] : kvs) {
        if (std::find(keysToRemove.begin(), keysToRemove.end(), key) != keysToRemove.end()) continue;

        auto ridOpt = tree->search(key);
        assert(ridOpt.has_value());
        std::string actual = heap->get(ridOpt.value());
        std::cout << key << " -> " << actual << "\n";
        assert(actual == expectedValue);
    }

    // === Final tree print ===
    std::cout << "\nFinal tree structure:\n";
    tree->print();

    std::cout << "\nHeap scan:\n";
    heap->scan([](uint64_t rid, const std::string& data) {
        std::cout << "RID " << rid << " -> " << data << "\n";
        });

    std::cout << "\n✅ Deletion tests passed!\n";

}
