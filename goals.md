🚀 PebbleDB — Project Goals & Roadmap

🎯 Big Picture:
✅ Build a lightweight embedded database engine in modern C++ (C++20),
✅ with reliable persistence, concurrency, and basic transactional support,
✅ similar in spirit to something like LevelDB, but educational & minimal.

📋 Phase-wise Goals
📑 Phase 1 — Minimal Viable Database
✅ Command-line program supporting basic CRUD:

put(key, value) → stores key–value pair persistently

get(key) → retrieves value for a given key

delete(key) → removes key from store

Data persists on disk (can shut down & restart and data is still there)

Implementation:

Single-threaded

Naive on-disk format (e.g., append-only log, or simple key-value file)

In-memory index or full scan to read keys

No transactions yet

Deliverable: a working .exe that survives restarts.



🔗 Phase 2 — Concurrency & Safety
✅ Support multiple threads accessing the DB safely:

Proper synchronization (mutexes/locks)

Thread-safe APIs

Thread pool for serving requests (optional)




📝 Phase 3 — Write-Ahead Log & Transactions
✅ Add atomicity & durability:

WAL (Write-Ahead Log) — writes are logged before applied

Crash-safe (can recover from log on restart)

Basic transactions (commit & rollback single operations)




🌲 Phase 4 — Index & Performance
✅ Replace naive file with a proper data structure:

B+Tree or SSTable-style file format

Sorted, efficient lookups

Optional caching layer (in-memory LRU)




🌎 Phase 5 — Optional / Stretch Goals
✅ Advanced features:

MVCC (Multi-Version Concurrency Control)

Replication (leader–follower, Raft, etc.)

Networking — expose a server that speaks some protocol (HTTP? gRPC?)

Query language or higher-level API