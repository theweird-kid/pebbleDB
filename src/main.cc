#include "CatalogManager.h"
#include "HeapFile.h"
#include "BufferPool.h"
#include "WindowsFileManager.h"

#include <iostream>

#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include "HeapFile.h" // Your implementation

void benchmark(HeapFile& hf, int N) {
    std::vector<uint64_t> recordIDs;

    auto start = std::chrono::high_resolution_clock::now();

    // Insert N records
    for (int i = 0; i < N; ++i) {
        std::string record = "Record_" + std::to_string(i);
        uint64_t rid = hf.insert(record);
        recordIDs.push_back(rid);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inserted " << N << " records in "
              << std::chrono::duration<double>(end - start).count() << " sec\n";

    // Get random records
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i += N / 10) {
        std::string r = hf.get(recordIDs[i]);
    }

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Read 10% of records in "
              << std::chrono::duration<double>(end - start).count() << " sec\n";

    // Scan all
    start = std::chrono::high_resolution_clock::now();

    hf.scan([](uint64_t rid, const std::string& data) {
        // Do nothing
    });

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Scanned all records in "
              << std::chrono::duration<double>(end - start).count() << " sec\n";

    // Delete half
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N / 2; ++i) {
        hf.remove(recordIDs[i]);
    }

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Deleted half the records in "
              << std::chrono::duration<double>(end - start).count() << " sec\n";
}


int main() {

    // Step 2: Setup file manager and buffer pool
    WindowsFileManager fileManager("users_data.db");
    BufferPool bufferPool(fileManager, 10);

    // Step 3: Open HeapFile
    HeapFile hf("users", bufferPool);

    // Step 4: Insert some rows
    auto rid1 = hf.insert("Alice");
    auto rid2 = hf.insert("Bob");
    auto rid3 = hf.insert("Charlie");

    std::cout << "\nInserted records:\n";
    std::cout << "RID1 = " << rid1 << "\n";
    std::cout << "RID2 = " << rid2 << "\n";
    std::cout << "RID3 = " << rid3 << "\n";

    // Step 5: Scan all records
    std::cout << "\nScanning all records:\n";
    hf.scan([](uint64_t rid, const std::string& rec) {
        std::cout << "RID: " << rid << " -> " << rec << "\n";
    });

    // Step 6: Delete RID2 (Bob)
    hf.remove(rid2);
    std::cout << "\nDeleted RID2 (Bob).\n";

    // Step 7: Scan again
    std::cout << "\nScanning records after deletion:\n";
    hf.scan([](uint64_t rid, const std::string& rec) {
        std::cout << "RID: " << rid << " -> " << rec << "\n";
    });

    benchmark(hf, 1000);

    return 0;
}
