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
	Block* insertEntryArray(DBenum index_type, 
							Block* root, 
							DBenum key_type, 
							const void* keys_void,
							size_t stride,
							const uint32_t* addrs, 
							int num){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		for(int i = 0; i < num; i++){
			executor->insert((const void*)((unsigned long)keys_void + i * stride), addrs[i]);
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
		executor->insert(key_void, addr);
		Block* ret = executor->getRoot();
		delete executor;
		return ret;
	}
	Block* removeEntry(DBenum index_type, 
						Block* root, 
						DBenum key_type,
						SearchResult* pos){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->remove(pos);
		Block* ret = executor->getRoot();
		delete executor;
		return ret;
	}
	SearchResult* searchEntry(DBenum index_type, 
								Block* root, 
								DBenum key_type, 
								const void* key_void){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		SearchResult* ret = executor->search(key_void);
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

	void destroySearchResult(SearchResult* & res){
		if(!res) return;
		else {
			if (res->node) {
				BufferManager::Instance().ReleaseBlock(res->node);
			}
			delete res;
			res = NULL;
		}
	}
	IndexExecutor* getIndexExecutor(DBenum index_type, Block* root, DBenum key_type){
		if(index_type == DB_BPTREE_INDEX) return new BPlusTree(root, key_type);
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
