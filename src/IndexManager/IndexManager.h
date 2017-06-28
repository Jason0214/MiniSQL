#pragma once
#include <vector>
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "BPlusTree.h"
#include "IndexMethod.h"
#include "../Type/ConstChar.h"

//virtual class to derive TypedIndexManager of different types
class IndexManager {
public:
	IndexManager() {};
	virtual ~IndexManager() {};
	virtual Block* insertEntryArray(Block* root, MethodType type, void* keys_void, uint32_t* addrs, int num) = 0;
	virtual Block* insertEntry(Block* root, MethodType type, void* key_void, uint32_t addr) = 0;
	virtual Block* removeEntry(Block* root, MethodType type, SearchResult* pos) = 0;
	virtual SearchResult* searchEntry(Block* root, MethodType type, void* key_void) = 0;
	virtual void removeIndex(Block* root, MethodType type) = 0;
	virtual void printAll(Block* root, MethodType type) = 0;
	virtual void initRootBlock(Block* block, MethodType type) = 0;
	virtual void writeAll() = 0;
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

//get index manager according to type
inline IndexManager* getIndexManager(DBenum type) {
	IndexManager* indexManager;
	if (type == DB_TYPE_INT) {
		indexManager = new TypedIndexManager<int>();
	}
	else if (type == DB_TYPE_FLOAT) {
		indexManager = new TypedIndexManager<float>();
	}
	else if (type - DB_TYPE_CHAR < 16) {
		indexManager = new TypedIndexManager<ConstChar<16>>();
	}
	else if (type - DB_TYPE_CHAR < 33) {
		indexManager = new TypedIndexManager<ConstChar<33>>();
	}
	else if (type - DB_TYPE_CHAR < 64) {
		indexManager = new TypedIndexManager<ConstChar<64>>();
	}
	else if (type - DB_TYPE_CHAR < 128) {
		indexManager = new TypedIndexManager<ConstChar<128>>();
	}
	else {
		indexManager = new TypedIndexManager<ConstChar<256>>();
	}
	return indexManager;
}