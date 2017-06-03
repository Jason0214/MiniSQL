#pragma once
#include<iostream>
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"

template<class T, int order>
class BPlusNode :public Block {
public:
	bool & isLeaf() {
		return *(bool*)(&block_data[BLOCK_HEAD_SIZE]);
	}
	uint32_t & parent() {
		return *(uint32_t*)(&block_data[BLOCK_HEAD_SIZE + 1]);
	}
	//unsigned int & order() {
	//return *(unsigned int*)(&block_data[BLOCK_HEAD_SIZE + 5]);
	//}
	int & dataCnt() {
		return *(int*)(&block_data[BLOCK_HEAD_SIZE + 9]);
	}
	T *data() {
		return (T*)(&block_data[BLOCK_HEAD_SIZE + 13]);
	}
	uint32_t *ptrs() {
		return (uint32_t*)(&block_data[BLOCK_HEAD_SIZE + 13 + order * sizeof(T)]);//not order-1: one more position for split easily
	}
};

//search result class
//contains the block and the index of the result
template<class T, int order>
class BPlusTreeSearchResult {
public:
	int index;
	BPlusNode<T, order>* node;
};

template<class T, int order>
class BPlusTree {
public:
	BPlusTree(BPlusNode<T, order> *root = nullptr, unsigned int keyLen = 25) :root(root), keyLen(keyLen) {
		bufferManager = &BufferManager::Instance();
	};
	virtual ~BPlusTree() {}
	//insert a entry into the b+tree
	void insert(T key, uint32_t addr) {
		//std::cout << key;
		BPlusNode<T, order>* theNode;
		// if the root hasn't been created
		if (!root) {
			theNode = root = static_cast<BPlusNode<T, order>*>(bufferManager->CreateBlock());
			theNode->isLeaf() = true;
			theNode->dataCnt() = 1;
			theNode->parent() = 0;
			theNode->data()[0] = key;
			theNode->ptrs()[0] = addr;
			theNode->ptrs()[order] = 0;
			return;
		}
		//normal case
		theNode = root;
		while (!theNode->isLeaf()) {
			theNode = static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(theNode->ptrs()[findLargerInBlock(key, theNode)]));
		}
		insertInBlock(key, addr, theNode);
	}
	//search according to the key
	//return a pointer to BPlusTreeSearchResult object
	//the result if the first matched key in the linked list
	//or the smallest larger one
	BPlusTreeSearchResult<T, order>* search(T key) {
		int index;
		BPlusNode<T, order>* theNode = root;
		//retunr null pointer if the tree is empty
		if (!root) {
			return nullptr;
		}
		while (!theNode->isLeaf()) {
			index = findFirstInBlock(key, theNode);
			theNode = static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(theNode->ptrs()[index]));
		}
		index = findFirstInBlock(key, theNode);
		BPlusTreeSearchResult<T, order>* result = new BPlusTreeSearchResult<T, order>();
		result->index = index;
		result->node = theNode;
		return result;
	}
	//print out all leaf nodes
	void printAll() {
		BPlusNode<T, order>* theNode = root;
		if (!theNode) return;
		while (!theNode->isLeaf()) {
			theNode = static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(theNode->ptrs()[0]));
		}
		while (true) {
			for (int i = 0;i < theNode->dataCnt();i++) {
				std::cout << theNode->data()[i] << " ";
			}
			if (theNode->ptrs()[order]) {
				theNode = static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(theNode->ptrs()[order]));
			}
			else {
				break;
			}
		}
		std::cout << std::endl;
	}
	//remove a entry from the b+tree
	void remove(T key, uint32_t addr);
	//remove the whole b+tree
	void removeAll();
protected:
	BPlusNode<T, order>* root;
	unsigned int keyLen;
	BufferManager* bufferManager;
	// insert a new entry
	void insertInBlock(T key, uint32_t addr, BPlusNode<T, order>* theNode) {
		//std::cout << '<'<<theNode->dataCnt() << '>';
		int idx = this->findLargerInBlock(key, theNode);
		if (theNode->isLeaf()) {
			for (int i = theNode->dataCnt() - 1;i >= idx;i--) {
				theNode->data()[i + 1] = theNode->data()[i];
				theNode->ptrs()[i + 1] = theNode->ptrs()[i];
			}
			theNode->data()[idx] = key;
			theNode->ptrs()[idx] = addr;
		}
		else {
			for (int i = theNode->dataCnt() - 1;i >= idx;i--) {
				theNode->data()[i + 1] = theNode->data()[i];
				theNode->ptrs()[i + 2] = theNode->ptrs()[i + 1];
			}
			theNode->data()[idx] = key;
			theNode->ptrs()[idx + 1] = addr;
		}
		theNode->dataCnt()++;
		//if needed, split the node
		if (theNode->dataCnt() >= order) {
			split(theNode);
		}
	}
	//find the smallest larger entry in the block
	int findLargerInBlock(T key, BPlusNode<T, order>* theNode) {
		int low = 0, mid, high = theNode->dataCnt() - 1;
		// binary search
		while (low <= high)
		{
			mid = (low + high) >> 1;
			T e = theNode->data()[mid];
			if (e < key) low = mid + 1;
			else if (e > key) high = mid - 1;
			else return mid;
		}

		// return low if key is not found
		return low;
	}
	//find the first matched key in the block
	int findFirstInBlock(T key, BPlusNode<T, order>* theNode) {
		int index = findLargerInBlock(key, theNode);
		//trace back the first matched key
		while (index > 0 && theNode->data()[index - 1] == key) index--;
		return index;
	}
	//find the data in the block
	int findInBlock(T key, BPlusNode<T, order>* theNode) {
		int low = 0, mid, high = theNode->dataCnt() - 1;

		// true binary search
		while (low <= high) {
			mid = (low + high) >> 1;
			T e = theNode->data()[mid];
			if (e < key) low = mid + 1;
			else if (e > key) high = mid - 1;
			else return mid;
		}
		// not found, return -1
		return -1;
	}
	//split the current node
	void split(BPlusNode<T, order>* theNode) {
		BPlusNode<T, order>* newNode = static_cast<BPlusNode<T, order>*>(bufferManager->CreateBlock());
		//if the node is the root
		if (!theNode->parent()) {
			//create a new root
			root = static_cast<BPlusNode<T, order>*>(bufferManager->CreateBlock());
			root->isLeaf() = false;
			root->dataCnt() = 0;
			root->ptrs()[0] = theNode->BlockIndex();
			root->parent() = 0;
			root->ptrs()[order] = 0;
			theNode->parent() = root->BlockIndex();
		}
		//leaf node
		if (theNode->isLeaf()) {
			memcpy(&(newNode->data()[0]), &(theNode->data()[order - order / 2]), order / 2 * sizeof(T));
			memcpy(&(newNode->ptrs()[0]), &(theNode->ptrs()[order - order / 2]), order / 2 * sizeof(T));
			newNode->isLeaf() = true;
			newNode->parent() = theNode->parent();
			newNode->dataCnt() = order / 2;
			theNode->dataCnt() = order - order / 2;
			//maintain linked list
			newNode->ptrs()[order] = theNode->ptrs()[order];
			theNode->ptrs()[order] = newNode->BlockIndex();
			//insert new entry to parent
			insertInBlock(newNode->data()[0], newNode->BlockIndex(), static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(newNode->parent())));
		}
		//non-leaf node
		else {
			memcpy(&(newNode->data()[0]), &(theNode->data()[order / 2 + 1]), (order - order / 2 - 1) * sizeof(T));
			memcpy(&(newNode->ptrs()[0]), &(theNode->ptrs()[order / 2 + 1]), (order - order / 2) * sizeof(T));
			newNode->isLeaf() = false;
			newNode->parent() = theNode->parent();
			newNode->dataCnt() = order - order / 2 - 1;
			theNode->dataCnt() = order / 2;
			//insert new entry to parent
			insertInBlock(theNode->data()[order / 2], newNode->BlockIndex(), static_cast<BPlusNode<T, order>*>(bufferManager->GetBlock(newNode->parent())));
		}
	}
};

