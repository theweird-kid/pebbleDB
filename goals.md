📜 Minimal SQL DB Roadmap
🧱 Phase 1: Foundation (you already started)
 Storage manager: pages, files, buffer pool

 WAL & recovery

 TransactionManager with begin, commit, abort

 Lock manager (basic exclusive/shared)

📖 Phase 2: Catalog & Metadata
 Create a Catalog Manager:

Stores definitions of tables & columns (schemas)

E.g., catalog.db file with:

pgsql
Copy
Edit
Table: users
Columns: id INT, name TEXT
SQL:

sql
Copy
Edit
CREATE TABLE users (id INT, name TEXT);
Add support to parse CREATE TABLE and record the schema.

🪄 Phase 3: SQL Parser
Build or integrate a SQL parser:

Simple hand-written parser → acceptable for now.

Support:

sql
Copy
Edit
CREATE TABLE …;
INSERT INTO … VALUES …;
SELECT … FROM … WHERE …;
UPDATE … SET … WHERE …;
DELETE FROM … WHERE …;
Parse query → AST (Abstract Syntax Tree)

⚙️ Phase 4: Query Planner & Executor
Given an AST:

Look up the table & schema in catalog

Use heap files & indexes to find data

Build an Execution Plan

E.g., TableScan → Filter → Projection

Execution:

TableScan: iterate through all pages of a table

Filter: apply WHERE conditions

Projection: return only selected columns

🗃️ Phase 5: Indexing
Build a B+ Tree index

Support CREATE INDEX

Use during WHERE lookups to avoid full table scans

🔁 Phase 6: Concurrency & Recovery
Finish TransactionManager:

Isolation levels: (just SERIALIZABLE or READ COMMITTED for now)

Implement Lock Manager (2PL → two-phase locking)

Ensure WAL + buffer pool flush + locks work correctly

Test recovery: replay WAL after crash

Optional (later):
Joins (nested loop join → hash join → sort-merge join)

Query optimizer (cost-based plan selection)

Aggregations (COUNT, SUM, etc.)

More advanced SQL (GROUP BY, ORDER BY)

🚀 Example of Supported SQL
At the end of Phase 4–5 you should be able to handle queries like:

sql
Copy
Edit
CREATE TABLE users (id INT, name TEXT);
INSERT INTO users VALUES (1, 'Alice');
INSERT INTO users VALUES (2, 'Bob');
SELECT id, name FROM users WHERE id = 1;
UPDATE users SET name = 'Charlie' WHERE id = 2;
DELETE FROM users WHERE id = 1;
📌 Next Step: TransactionManager
Before you move to SQL parsing & catalog, finish the TransactionManager, so you have clean support for BEGIN, COMMIT, ABORT, and WAL integration.

