#pragma once
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"

class IndexManager{
public:
	static IndexManager & Instance(){
		static IndexManager theIndexManager;
		return theIndexManager;
	}
	~IndexManager() {};
	uint32_t insertEntryArray(DBenum index_type, 
							uint32_t root_addr, 
							DBenum key_type, 
							const void* keys_void,
							size_t stride,
							const uint32_t* addrs, 
							int num){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		for(int i = 0; i < num; i++){
			executor->insert((const void*)((unsigned long)keys_void + i * stride), addrs[i]);
		}
		uint32_t ret = executor->getRoot();
		delete executor;
		return ret;
	}
	uint32_t insertEntry(DBenum index_type,
						uint32_t root_addr, 
						DBenum key_type,
						const void* key_void, 
						uint32_t addr){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		executor->insert(key_void, addr);
		uint32_t ret = executor->getRoot();
		delete executor;
		return ret;
	}
	uint32_t removeEntry(DBenum index_type, 
						uint32_t root_addr,  
						DBenum key_type,
						SearchResult* pos){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		executor->remove(pos);
		uint32_t ret = executor->getRoot();
		delete executor;
		return ret;
	}
	SearchResult* searchEntry(DBenum index_type, 
								uint32_t root_addr,  
								DBenum key_type, 
								const void* key_void){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		SearchResult* ret = executor->search(key_void);
		delete executor;
		return ret;
	}
	void removeIndex(DBenum index_type, uint32_t root_addr, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		executor->removeAll();
		delete executor;		
	}
	void printAll(DBenum index_type, uint32_t root_addr, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		executor->printAll();
		delete executor;
	}
	void initRootBlock(DBenum index_type, uint32_t root_addr, DBenum key_type){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root_addr, key_type);
		executor->initBlock(root_addr);
		delete executor;
	}
	void writeAll(){

	}

	IndexExecutor* getIndexExecutor(DBenum index_type, uint32_t root_addr, DBenum key_type){
		if(index_type == DB_BPTREE_INDEX) return new BPlusTree(root_addr, key_type);
		else {
			return NULL;
		}
		//TODO hash index
	}
private:
	IndexManager(){};
	IndexManager(const IndexManager &);
	IndexManager & operator=(const IndexManager &);
};
