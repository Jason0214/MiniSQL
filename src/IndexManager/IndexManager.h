#pragma once
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"

enum IndexType {
	BPTree = 0
};

//search result class
//contains the block and the index of the result
class SearchResult {
public:
	int index;
	Block* node;
};

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
	//create a new index on a table
	Block* createIndex(IndexType type, T* keys, uint32_t* addrs, unsigned int num, unsigned int keyLen) {
		IndexMethod<T>* method = createMethod(type, keyLen, nullptr);
		for (int i = 0;i < num;i++) {
			//insert entries
			method->insert(keys[i], addrs[i]);
		}
		Block root = method->getRoot();
		delete method; //destructor will write data to disk
		return root;
	}
	//remove an existing index from a table
	void removeIndex(IndexType type, Block* root, unsigned int keyLen) {
		IndexMethod<T>* method = createMethod(type, keyLen,root);
		method->removeAll();
		delete method;
	}
protected:
	IndexMethod<T>* createMethod(IndexType type, unsigned int keyLen,Block* root) {
		if (type == BPTree) {
			if (keyLen <= 4) {
				method = new BPlusTree<int, 508> bptree(root, 508);
			}
			else {
				method = new BPlusTree<int, 508> bptree(root, 508);
			}
		}
		return method;
	}
};