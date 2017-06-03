#pragma once
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"

enum IndexType {
	BPTree = 0
};

class IndexManager {
protected:
	IndexManager();
	virtual ~IndexManager();
	Block* createIndex(IndexType type,Block* table,unsigned int keyLength);
	void removeIndex(Block* root);
};