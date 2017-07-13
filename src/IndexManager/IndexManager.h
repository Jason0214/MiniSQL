#pragma once
#include <vector>
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"
#include "IndexExecutor.h"


class IndexManager{
public:
	static IndexManager & Instance(){
		static IndexManager theIndexManager;
		return theIndexManager;
	}
	~IndexManager() {};
	Block* insertEntryArray(DBenum index_type, 
							Block* root, 
							DBenum key_type, 
							const void* keys_void,
							size_t stride,
							const uint32_t* addrs, 
							int num){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		for(int i = 0; i < num; i++){
			executor->insert(keys_void + i * stride, addrs[i]);
		}
		Block* ret = executor->getRoot();
		delete executor;
		return ret;
	}
	Block* insertEntry(DBenum index_type, 
						Block* root, 
						DBenum key_type, 
						const void* key_void, 
						uint32_t addr){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		Block* ret = executor->insert(key_void, addr);
		delete executor;
		return ret;
	}
	Block* removeEntry(DBenum index_type, 
						Block* root, 
						DBenum key_type, 
						SearchResult* pos){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		Block* ret = executor->remove(pos);
		delete executor;
		return ret;
	}
	SearchResult* searchEntry(DBenum index_type, 
								Block* root, 
								DBenum key_type, 
								const void* key_void){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		SearchResult* ret = executor->insert(key_void);
		delete executor;
		return ret;
	}
	void removeIndex(DBenum index_type, Block* root, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->removeAll();
		delete executor;		
	}
	void printAll(DBenum index_type, Block* root, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->printAll();
		delete executor;
	}
	void initRootBlock(DBenum index_type, Block* root, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->initBlock(root);
		delete executor;
	}
	void writeAll(){

	}

	void destroySearchResult(SearchResult* ){

	}
private:
	IndexManager();
	IndexManager(const IndexManager &);
	IndexManager & operator=(const IndexManager &);

	IndexExecutor* getIndexExecutor(DBenum index_type, Block* root, DBenum key_type){
		if(index_type == DB_BPTREE_INDEX) return new BPlusTree(root, key_type);
		//TODO hash index
	}
};
