#pragma once

#include "pebble/core/CatalogManager.h"
#include "pebble/core/HeapFile.h"
#include "pebble/core/BPlusTree.h"
#include "pebble/core/BufferPool.h"
#include "pebble/core/IFileManager.h"
#include "pebble/core/WindowsFileManager.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

namespace pebble {
	namespace app {

		class StorageEngine
		{
		public:
			StorageEngine(const std::string& dbPath, size_t poolSize);

			// Collection Management
			bool createCollection(const std::string& name);
			bool dropCollection(const std::string& name);
			void showCollections() const;

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
			pebble::core::WindowsFileManager m_FileManager;
			pebble::core::BufferPool m_BufferPool;
			pebble::core::CatalogManager m_CatalogManager;

			struct Collection {
				std::unique_ptr<pebble::core::HeapFile> heap;
				std::unique_ptr<pebble::core::BPlusTree> index;
			};

			std::unordered_map<std::string, Collection> collections;

			Collection* loadCollection(const std::string& name);
		};
	}
}