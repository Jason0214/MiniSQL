#pragma once
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"

//indexing type
enum MethodType {
	BPTree = 0
};

//search result class
//contains the block and the index of the result
class SearchResult {
public:
	int index;
	Block* node;
};

//virtual class to derive indexing methods like b+tree
template<class T>
class IndexMethod {
public:
	IndexMethod() {};
	virtual ~IndexMethod() {};
	virtual void insert(T key, uint32_t addr) = 0;
	virtual SearchResult* search(T key) = 0;
	virtual void printAll() = 0;
	virtual Block* getRoot() = 0;
	virtual void remove(SearchResult* pos)=0;
	virtual void removeAll()=0;
};

template<class T>
class IndexManager {
public:
	IndexManager() {};
	virtual ~IndexManager() {};
	//insert an array of entries
	//use root = nullptr to create a new index
	Block* insertEntryArray(Block* root, MethodType type, T* keys, uint32_t* addrs, unsigned int num) {
		IndexMethod<T>* method = createMethod(type, root);
		for (int i = 0;i < num;i++) {
			//insert entries
			method->insert(keys[i], addrs[i]);
		}
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
	SearchResult* searchEntry(Block* root, MethodType type, T key) {
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