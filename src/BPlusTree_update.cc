#include "pebble/core/BPlusTree.h"

using namespace pebble::core;

bool BPlusTree::update(int key, uint64_t newValue)
{
	if(m_RootPageID == 0) {
		return false; // Tree is empty
	}
	return updateInternal(key, newValue, m_RootPageID);
}

bool BPlusTree::updateInternal(int key, uint64_t newValue, uint32_t pageID)
{
	return false;
}