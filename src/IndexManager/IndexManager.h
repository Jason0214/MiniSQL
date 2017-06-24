#pragma once
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"
#include "IndexMethod.h"


//virtual class to derive TypedIndexManager of different types
class IndexManager {
public:
	IndexManager() {};
	virtual ~IndexManager() {};
	virtual Block* insertEntryArray(Block* root, MethodType type, void* keys_void, uint32_t* addrs, unsigned int num) = 0;
	virtual Block* insertEntry(Block* root, MethodType type, void* key_void, uint32_t addr) = 0;
	virtual Block* removeEntry(Block* root, MethodType type, SearchResult* pos) = 0;
	virtual SearchResult* searchEntry(Block* root, MethodType type, void* key_void) = 0;
	virtual void removeIndex(Block* root, MethodType type) = 0;
	virtual void printAll(Block* root, MethodType type) = 0;
	virtual void initRootBlock(Block* block, MethodType type) = 0;
};

template<class T>
class TypedIndexManager:public IndexManager {
public:
	TypedIndexManager() {};
	virtual ~TypedIndexManager() {};
	//insert an array of entries
	//use root = nullptr to create a new index
	Block* insertEntryArray(Block* root, MethodType type, void* keys_void, uint32_t* addrs, unsigned int num) {
		T* keys = (T*)(keys_void);
		IndexMethod<T>* method = createMethod(type, root);
		for (int i = 0;i < num;i++) {
			//insert entries
			method->insert(keys[i], addrs[i]);
		}
		root = method->getRoot();
		delete method; //destructor will write data to disk
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
		delete method; //destructor will write data to disk
		return root;
	}
	//remove an entry from existing index
	Block* removeEntry(Block* root, MethodType type, SearchResult* pos) {
		IndexMethod<T>* method = createMethod(type, root);
		method->remove(pos);
		root = method->getRoot();
		delete method; //destructor will write data to disk
		return root;
	}
	//search an entry in existing index
	SearchResult* searchEntry(Block* root, MethodType type, void* key_void) {
		T key = *(T*)(key_void);
		IndexMethod<T>* method = createMethod(type, root);
		SearchResult* result = method->search(key);
		delete method; //destructor will write data to disk
		return result;
	}
	//remove an existing index from a table
	void removeIndex(Block* root, MethodType type) {
		IndexMethod<T>* method = createMethod(type,root);
		method->removeAll();
		delete method;
	}
	//print out the data in the index
	void printAll(Block* root, MethodType type) {
		IndexMethod<T>* method = createMethod(type, root);
		method->printAll();
		delete method;
	}
	void initRootBlock(Block* block, MethodType type) {
		IndexMethod<T>* method = createMethod(type, root);
		method->initBlock(block);
		delete method;
	}
protected:
	IndexMethod<T>* createMethod(MethodType type,Block* root) {
		IndexMethod<T>* method;
		if (type == BPTree) {
			//BLOCK_HEAD_SIZE + BP_HEAD_SIZE + order * SIZEOF(T) + 4 * (order + 1)<=4096
			method = new BPlusTree<T>(root, (4096 - BLOCK_HEAD_SIZE - BP_HEAD_SIZE - 4) / (sizeof(uint32_t*) + sizeof(T)));
		}
		return method;
	}
};