#include "StorageEngine.h"
#include <iostream>

StorageEngine::StorageEngine(const std::string& dbPath, size_t poolSize)
	: m_FileManager(dbPath), 
	m_BufferPool(m_FileManager, poolSize),
	m_CatalogManager(m_BufferPool)
{}

bool StorageEngine::createCollection(const std::string& name)
{
	// Allocate root page for B+ Tree rootNode and HeapFile start page
	PageID rootPage = m_BufferPool.allocatePage();
	PageID heapPage = m_BufferPool.allocatePage();

	// register in catalog
	m_CatalogManager.createCollection(name, rootPage, heapPage);

	// Load into memory
	Collection col;
	col.index = std::make_unique<BPlusTree>(m_BufferPool, rootPage);
	col.heap = std::make_unique<HeapFile>(m_BufferPool, heapPage);
	collections[name] = std::move(col);

	return true;
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
	return col->index->insert(key, rid);
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