#ifndef _INDEX_EXECUTOR_
#define _INDEX_EXECUTOR_


//search result class
//contains the block and the index of the result
class SearchResult {
public:
	int index;
	int dataLen;
	void* data;
	uint32_t* ptrs;
	Block* node;
};


class IndexExecutor {
public:
	IndexExecutor() {};
	virtual ~IndexExecutor() {};
	virtual void insert(const void* key, uint32_t addr) = 0;
	virtual SearchResult* search(const void* key) = 0;
	virtual void printAll() = 0;
	virtual Block* getRoot() = 0;
	virtual void remove(SearchResult* pos) = 0;
	virtual void removeAll() = 0;
	virtual void initBlock(Block* block) = 0;
};

#endif