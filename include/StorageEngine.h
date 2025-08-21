#pragma once

#include "CatalogManager.h"
#include "HeapFile.h"
#include "BPlusTree.h"
#include "BufferPool.h"
#include "IFileManager.h"
#include "WindowsFileManager.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

class StorageEngine
{
public:
	StorageEngine(const std::string& dbPath, size_t poolSize);

	// Collection Management
	bool createCollection(const std::string& name);
	bool dropCollection(const std::string& name);

	// CRUD
	bool insert(const std::string& collection, int key, const std::string& value);
	std::optional<std::string> get(const std::string& collection, int key);
	bool update(const std::string& collection, int key, const std::string& newValue);
	bool remove(const std::string& collection, int key);

	// DEBUG / Utility
	void printTree(const std::string& collection);
	void printHeap(const std::string& collection);
	void printFreeList();

private:
	WindowsFileManager m_FileManager;
	BufferPool m_BufferPool;
	CatalogManager m_CatalogManager;

	struct Collection {
		std::unique_ptr<HeapFile> heap;
		std::unique_ptr<BPlusTree> index;
	};

	std::unordered_map<std::string, Collection> collections;

	Collection* loadCollection(const std::string& name);
};