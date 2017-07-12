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
		delete executor;
	}
	Block* insertEntry(DBenum index_type, 
						Block* root, 
						DBenum key_type, 
						const void* key_void, 
						uint32_t addr){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->insert(key_void, addr);
		delete executor;
	}
	Block* removeEntry(DBenum index_type, 
						Block* root, 
						DBenum key_type, 
						SearchResult* pos){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->remove(pos);
		delete executor;
	}
	SearchResult* searchEntry(DBenum index_type, 
								Block* root, 
								DBenum key_type, 
								const void* key_void){
		IndexExecutor* executor = this->getIndexExecutor(index_type, root, key_type);
		executor->insert(key_void);
		delete executor;
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
private:
	IndexManager();
	IndexManager(const IndexManager &);
	IndexManager & operator=(const IndexManager &);

	IndexExecutor* getIndexExecutor(DBenum index_type, Block* root, DBenum key_type){
		if(index_type == DB_BPTREE_INDEX) return new BPlusTree(root, key_type);
		//TODO hash index
	}
};



template<class T>
class TypedIndexManager:public IndexManager {
public:
	TypedIndexManager() {};
	virtual ~TypedIndexManager() {
		for (auto i = methods.begin();i < methods.end();i++) {
			delete *i; //destructor will write data to disk
		}
	};
	//insert an array of entries
	//use root = nullptr to create a new index
	Block* insertEntryArray(Block* root, MethodType type, void* keys_void, uint32_t* addrs, int num) {
		T* keys = (T*)(keys_void);
		IndexMethod<T>* method = createMethod(type, root);
		for (int i = 0;i < num;i++) {
			//insert entries
			method->insert(keys[i], addrs[i]);
		}
		root = method->getRoot();
		return root;
	}
	//insert an  entry
	//use root = nullptr to create a new index
	Block* insertEntry(Block* root, MethodType type, void* key_void, uint32_t addr) {
		T key = *(T*)(key_void);
		IndexMethod<T>* method = createMethod(type, root);
		//insert entry
		method->insert(key, addr);
		root = method->getRoot();
		return root;
	}
	//remove an entry from existing index
	Block* removeEntry(Block* root, MethodType type, SearchResult* pos) {
		IndexMethod<T>* method = createMethod(type, root);
		method->remove(pos);
		root = method->getRoot();
		return root;
	}
	//search an entry in existing index
	SearchResult* searchEntry(Block* root, MethodType type, void* key_void) {
		T key = *(T*)(key_void);
		IndexMethod<T>* method = createMethod(type, root);
		SearchResult* result = method->search(key);
		return result;
	}
	//remove an existing index from a table
	void removeIndex(Block* root, MethodType type) {
		IndexMethod<T>* method = createMethod(type,root);
		method->removeAll();
	}
	//print out the data in the index
	void printAll(Block* root, MethodType type) {
		IndexMethod<T>* method = createMethod(type, root);
		method->printAll();
	}
	void initRootBlock(Block* root, MethodType type) {
		IndexMethod<T>* method = createMethod(type, root);
		method->initBlock(root);
	}
	virtual void writeAll() {
		for (auto i = methods.begin();i < methods.end();i++) {
			delete *i; //destructor will write data to disk
		}
		methods.empty();//clean methods vector
	}
protected:
	IndexMethod<T>* createMethod(MethodType type,Block* root) {
		//check if the method already exists
		for (auto i = methods.begin();i < methods.end();i++) {
			if ((*i)->getRoot() == root) {
				return *i;
			}
		}
		IndexMethod<T>* method;
		if (type == BPTree) {
			//BLOCK_HEAD_SIZE + BP_HEAD_SIZE + order * SIZEOF(T) + 4 * (order + 1)<=4096
			int order = (BLOCK_SIZE - BLOCK_HEAD_SIZE - BP_HEAD_SIZE - (sizeof(uint32_t*))) / (sizeof(uint32_t*) + sizeof(T));
			method = new BPlusTree<T>(root, order);
		}
		methods.push_back(method);
		return method;
	}
	//store methods
	std::vector<IndexMethod<T>*> methods;
};