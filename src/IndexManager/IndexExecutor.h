#ifndef _INDEX_EXECUTOR_
#define _INDEX_EXECUTOR_

#include "../BufferManager/BlockPtr.h"

//search result class
//contains the block and the index of the result
class SearchResult {
public:
	SearchResult(BPlusNode* bp_node, int index)
	:index(index), node(bp_node){}
	int index;
	BlockPtr<BPlusNode> node;
};


class IndexExecutor {
public:
	IndexExecutor() {};
	virtual ~IndexExecutor() {};
	virtual void insert(const void* key, uint32_t addr) = 0;
	virtual SearchResult* search(const void* key) = 0;
	virtual void printAll() = 0;
	virtual uint32_t getRoot() = 0;
	virtual void remove(SearchResult* pos) = 0;
	virtual void removeAll() = 0;
	virtual void initBlock(uint32_t block_addr) = 0;
};

#endif