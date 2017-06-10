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
	virtual void remove(T key, uint32_t addr)=0;
	virtual void removeAll()=0;
};


template<class T>
class IndexManager {
public:
	IndexManager() {};
	virtual ~IndexManager() {};
	Block* createIndex(IndexType type, T* keys, uint32_t* addrs, unsigned int num, unsigned int keyLen) {
		IndexMethod<T> method;
		if (type == BPTree) {
			if (keyLen <= 4) {
				method = new BPlusTree<int, 508> bptree(nullptr, 508);
			}
			else {
				method = new BPlusTree<int, 508> bptree(nullptr, 508);
			}
			for (int i = 0;i < num;i++) {
				//insert entry into b+tree
				method.insert(keys[i], addrs[i]);
			}
		}
		return method.getRoot();
	}
	void removeIndex(Block* root);
};