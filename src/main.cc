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

    /*for (auto& [key, value] : kvs) {
        uint64_t rid = heap->insert(value);
        bool split = tree->insert(key, rid);
        if(split) {
            std::cout << "Tree split occurred after inserting key: " << key << "\n";
            cm.updateCollectionMeta(collectionName, 
                tree->rootPageID(), 
                heap->getStartPageID()
			);
		}
        std::cout << "Inserted: " << key << " => " << value << "\n";
    }*/

    std::cout << "\nVerifying lookups:\n";
    for (auto& [key, expectedValue] : kvs) {
        auto ridOpt = tree->search(key);
        assert(ridOpt.has_value());
        std::string actual = heap->get(ridOpt.value());
        std::cout << key << " -> " << actual << "\n";
        assert(actual == expectedValue);
    }


    std::cout << "\nScan all remaining records:\n";
    heap->scan([](uint64_t rid, const std::string& data) {
        std::cout << "RID " << rid << " -> " << data << "\n";
    });

	tree->print();

    delete tree;
    delete heap;

    std::cout << "\n✅ All tests passed!\n";
    return 0;
}
