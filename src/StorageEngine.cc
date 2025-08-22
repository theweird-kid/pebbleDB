#include "pebble/app/StorageEngine.h"
#include <iostream>

using namespace pebble::app;

StorageEngine::StorageEngine(const std::string& dbPath, size_t poolSize)
	: m_FileManager(dbPath), 
	m_BufferPool(m_FileManager, poolSize),
	m_CatalogManager(m_BufferPool)
{}

bool StorageEngine::createCollection(const std::string& name) {
	// Check if already loaded in memory
	if (collections.find(name) != collections.end()) {
		return false; // already exists in this process
	}

	// Ask catalog
	auto meta = m_CatalogManager.getCollectionMeta(name);
	if (meta) {
		// Already exists on disk → load it
		auto [rootPageID, heapStartPageID] = *meta;

		Collection coll;
		coll.heap = std::make_unique<pebble::core::HeapFile>(name, m_BufferPool, heapStartPageID);
		coll.index = std::make_unique<pebble::core::BPlusTree>(m_BufferPool, rootPageID);
		collections[name] = std::move(coll);

		return false; // means "already existed"
	}
	else {
		// Create fresh
		auto heap = std::make_unique<pebble::core::HeapFile>(name, m_BufferPool);
		auto index = std::make_unique<pebble::core::BPlusTree>(m_BufferPool);

		uint32_t heapStartPageID = heap->getStartPageID();
		uint32_t rootPageID = index->rootPageID();

		m_CatalogManager.createCollection(name, rootPageID, heapStartPageID);

		Collection coll{ std::move(heap), std::move(index) };
		collections[name] = std::move(coll);

		return true; // new collection created
	}
}


bool StorageEngine::dropCollection(const std::string& name)
{
	auto it = collections.find(name);
	if (it == collections.end()) {
		auto meta = m_CatalogManager.getCollectionMeta(name);
		if (!meta) return false;	// Collection doesn't exist
		// TODO: free all pages from B+Tree and HeapFile 
	}
	else {
		// TODO: free in memory and on - disk pages
		collections.erase(it);
	}

	// For now we just remove from catalog (lazy GC of pages)
	m_CatalogManager.updateCollectionMeta(name, 0, 0);
	return true;
}

bool StorageEngine::insert(const std::string& collection, int key, const std::string& value)
{
	Collection* col = loadCollection(collection);
	if (!col)
		return false;

	// Insert into HeapFile -> returns recordID ( pageID, slotID )
	auto rid = col->heap->insert(value);

	// Insert into B+ Tree
	bool split = col->index->insert(key, rid);
	if (split) {
		std::cout << "Tree split occurred after inserting key: " << key << "\n";
		m_CatalogManager.updateCollectionMeta(collection, col->index->rootPageID(), col->heap->getStartPageID());
	}
	return true;
}

std::optional<std::string> StorageEngine::get(const std::string& collection, int key)
{
	Collection* col = loadCollection(collection);
	if (!col)
		return std::nullopt;

	auto ridOpt = col->index->search(key);
	if (!ridOpt)
		return std::nullopt;

	return col->heap->get(*ridOpt);
}

bool StorageEngine::update(const std::string& collection, int key, const std::string& newValue)
{
	Collection* col = loadCollection(collection);
	if (!col)
		return false;

	auto ridOpt = col->index->search(key);
	if (!ridOpt)
		return false;

	// Remove old Value and create new
	col->heap->remove(*ridOpt);
	auto newRid = col->heap->insert(newValue);
	return col->index->update(key, newRid);
}

bool StorageEngine::remove(const std::string& collection, int key)
{
	Collection* col = loadCollection(collection);
	if (!col)
		return false;

	auto ridOpt = col->index->search(key);
	if (!ridOpt)
		return false;

	col->heap->remove(*ridOpt);
	col->index->remove(key);
	return true;
}

void StorageEngine::printTree(const std::string& collection)
{
	Collection* col = loadCollection(collection);
	if (col) col->index->print();
}

void StorageEngine::printHeap(const std::string& collection)
{
	Collection* col = loadCollection(collection);
	//if (col) col->heap->print();
}

void StorageEngine::printFreeList()
{
	m_FileManager.printFreeList();
}

StorageEngine::Collection* StorageEngine::loadCollection(const std::string& name)
{
	auto it = collections.find(name);
	if (it != collections.end())
	{
		return &it->second;
	}

	auto metaOpt = m_CatalogManager.getCollectionMeta(name);
	if (!metaOpt)
		return nullptr;

	auto &[rootPage, heapPage] = *metaOpt;

	Collection col;
	col.index = std::make_unique<pebble::core::BPlusTree>(m_BufferPool, rootPage);
	col.heap = std::make_unique<pebble::core::HeapFile>(name, m_BufferPool, heapPage);

	collections[name] = std::move(col);
	return &collections[name];
}