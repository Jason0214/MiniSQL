#pragma once

#define BP_HEAD_SIZE 17

#include<iostream>
#include<cmath>
#include "../BufferManager/Block.h"
#include "../BufferManager/BufferManager.h"
#include "IndexMethod.h"

template<class T>
class BPlusNode :public Block {
public:
	inline bool & isLeaf() {
		return *(bool*)(&block_data[BLOCK_HEAD_SIZE]);
	}
	inline uint32_t & parent() {
		return *(uint32_t*)(&block_data[BLOCK_HEAD_SIZE + 1]);
	}
	inline uint32_t & rightSibling() {
		return *(uint32_t*)(&block_data[BLOCK_HEAD_SIZE + 5]);
	}
	inline int & order() {
		return *(int*)(&block_data[BLOCK_HEAD_SIZE + 9]);
	}
	inline int & dataCnt() {
		return *(int*)(&block_data[BLOCK_HEAD_SIZE + 13]);
	}
	inline T *data() {
		return (T*)(&block_data[BLOCK_HEAD_SIZE + 17]);
	}
	inline uint32_t *ptrs() {
		return (uint32_t*)(&block_data[BLOCK_HEAD_SIZE + 17 + order() * sizeof(T)]);//not order-1: one more position for split easily
	}
};

template<class T>
class BPlusTree : public IndexMethod<T> {
public:
	BPlusTree(Block *root, int order) : order(order) {
		this->root = static_cast<BPlusNode<T>*>(root);
		bufferManager = &BufferManager::Instance();
	};
	virtual ~BPlusTree() {
		operateTree(root, &BufferManager::ReleaseBlock);
	}
	//insert a entry into the b+tree
	void insert(T key, uint32_t addr) {
		BPlusNode<T>* theNode;
		// if the root hasn't been created
		if (!root) {
			theNode = root = static_cast<BPlusNode<T>*>(bufferManager->CreateBlock());
			theNode->isLeaf() = true;
			theNode->order() = order;
			theNode->dataCnt() = 1;
			theNode->parent() = 0;
			theNode->data()[0] = key;
			theNode->ptrs()[1] = addr;
			theNode->rightSibling() = 0;
			return;
		}
		//normal case
		theNode = root;
		while (!theNode->isLeaf()) {
			theNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->ptrs()[findLargerInBlock(key, theNode)]));
		}
		insertInBlock(key, addr, theNode);
	}
	//search according to the key
	//return a pointer to BPlusTreeSearchResult object
	//the result if the first matched key in the linked list
	//or the smallest larger one
	SearchResult* search(T key) {
		int index;
		BPlusNode<T>* theNode = root;
		//retunr null pointer if the tree is empty
		if (!root||!root->dataCnt()) {
			return nullptr;
		}
		while (!theNode->isLeaf()) {
			index = findFirstInBlock(key, theNode);
			theNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->ptrs()[index]));
		}
		index = findFirstInBlock(key, theNode);
		SearchResult* result = new SearchResult();
		result->index = index;
		result->node = theNode;
		return result;
	}
	//print out all leaf nodes
	void printAll() {
		BPlusNode<T>* theNode = root;
		if (!theNode) return;
		while (!theNode->isLeaf()) {
			theNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->ptrs()[0]));
		}
		while (true) {
			for (int i = 0;i < theNode->dataCnt();i++) {
				std::cout << theNode->data()[i] << " ";
			}
			if (theNode->rightSibling()) {
				theNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->rightSibling()));
			}
			else {
				break;
			}
		}
		std::cout << std::endl;
	}
	//get root block pointer
	Block* getRoot() {
		return root;
	}
	//remove a entry from the b+tree
	//pos must be in the tree or an error will occur
	void remove(SearchResult* pos) {
		BPlusNode<T>* theNode = static_cast<BPlusNode<T>*>(pos->node);
		removeInBlock(theNode, pos->index);
	}
	//remove the whole b+tree
	void removeAll() {
		operateTree(root, &BufferManager::DeleteBlock);
		root = nullptr;
	}
	//initialize a block (always the root node)
	void initBlock(Block* block) {
		BPlusNode<T>* theNode = static_cast<BPlusNode<T>*>(block);
		theNode->isLeaf() = true;
		theNode->order() = order;
		theNode->dataCnt() = 0;
		theNode->parent() = 0;
		theNode->rightSibling() = 0;
	}
protected:
	BPlusNode<T>* root;
	int order;
	BufferManager* bufferManager;
	// insert a new entry
	void insertInBlock(T key, uint32_t addr, BPlusNode<T>* theNode) {
		//std::cout << '<'<<theNode->dataCnt() << '>';
		int idx = this->findLargerInBlock(key, theNode);
		for (int i = theNode->dataCnt() - 1;i >= idx;i--) {
			theNode->data()[i + 1] = theNode->data()[i];
			theNode->ptrs()[i + 2] = theNode->ptrs()[i + 1];
		}
		theNode->data()[idx] = key;
		theNode->ptrs()[idx + 1] = addr;

		theNode->dataCnt()++;
		//if needed, split the node
		if (theNode->dataCnt() >= order) {
			split(theNode);
		}
	}
	//find the smallest larger entry in the block
	int findLargerInBlock(T key, BPlusNode<T>* theNode) {
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
	int findFirstInBlock(T key, BPlusNode<T>* theNode) {
		int index = findLargerInBlock(key, theNode);
		//trace back the first matched key
		while (index > 0 && theNode->data()[index - 1] == key) index--;
		return index;
	}
	//split the current node
	void split(BPlusNode<T>* theNode) {
		BPlusNode<T>* newNode = static_cast<BPlusNode<T>*>(bufferManager->CreateBlock());
		newNode->order() = order;
		//maintain linked list
		newNode->rightSibling() = theNode->rightSibling();
		theNode->rightSibling() = newNode->BlockIndex();
		//if the node is the root
		if (theNode==root) {
			//create a new root
			root = static_cast<BPlusNode<T>*>(bufferManager->CreateBlock());
			root->order() = order;
			root->isLeaf() = false;
			root->dataCnt() = 0;
			root->ptrs()[0] = theNode->BlockIndex();
			root->parent() = 0;
			root->rightSibling() = 0;
			theNode->parent() = root->BlockIndex();
		}
		//leaf node
		if (theNode->isLeaf()) {
			//copy data to new node
			memcpy(&(newNode->data()[0]), &(theNode->data()[order - order / 2]), order / 2 * sizeof(T));
			memcpy(&(newNode->ptrs()[1]), &(theNode->ptrs()[order - order / 2 + 1]), order / 2 * sizeof(T));
			newNode->isLeaf() = true;
			newNode->parent() = theNode->parent();
			newNode->dataCnt() = order / 2;
			theNode->dataCnt() = order - order / 2;
			//insert new entry to parent
			insertInBlock(newNode->data()[0], newNode->BlockIndex(), static_cast<BPlusNode<T>*>(bufferManager->GetBlock(newNode->parent())));
		}
		//non-leaf node
		else {
			//copy data to new node
			memcpy(&(newNode->data()[0]), &(theNode->data()[order / 2 + 1]), (order - order / 2 - 1) * sizeof(T));
			memcpy(&(newNode->ptrs()[0]), &(theNode->ptrs()[order / 2 + 1]), (order - order / 2) * sizeof(T));
			newNode->isLeaf() = false;
			newNode->parent() = theNode->parent();
			newNode->order() = order;
			newNode->dataCnt() = order - order / 2 - 1;
			theNode->dataCnt() = order / 2;
			//insert new entry to parent
			insertInBlock(theNode->data()[order / 2], newNode->BlockIndex(), static_cast<BPlusNode<T>*>(bufferManager->GetBlock(newNode->parent())));
		}
	}
	//delete an entry from the block
	void removeInBlock(BPlusNode<T>* theNode, unsigned int index) {
		for (int i = index;i < theNode->dataCnt() - 1;i++) {
			theNode->data()[i] = theNode->data()[i + 1];
			theNode->ptrs()[i + 1] = theNode->ptrs()[i + 2];
		}
		theNode->dataCnt()--;
		//check if there's too few data
		int minCnt = getMinCnt(theNode);
		if (!theNode->dataCnt() && theNode == root && theNode->isLeaf()) {
			return;
		}
		else if (theNode->dataCnt() < minCnt) {
			merge(theNode);
		}
	}
	//merge the current node to other node
	void merge(BPlusNode<T>* theNode) {
		BPlusNode<T>* rightNode;
		//if the node is the root, let its only child to be the new root
		if (theNode==root) {
			Block* oldRoot = root;
			//if root is a leaf, empty the tree
			if (theNode->isLeaf()) root = nullptr;
			//assign new root
			else root = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->ptrs()[0]));
			//delete the block
			bufferManager->DeleteBlock(oldRoot);
			return;
		}
		BPlusNode<T>* parentNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->parent()));
		//if the node is the last child of parent node, assign the node to be its left sibling
		if (parentNode->ptrs()[parentNode->dataCnt()] == theNode->BlockIndex()) {
			rightNode = theNode;
			theNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(parentNode->ptrs()[parentNode->dataCnt() - 1]));
		}
		//normally right node is the right sibling of the node
		else {
			rightNode = static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->rightSibling()));
		}
		int totalCnt = theNode->dataCnt() + rightNode->dataCnt();
		//cannot put in one node, redistribute entries
		if (totalCnt > order - 1) {
			T oldKey = rightNode->data()[0];
			int offset = totalCnt / 2 - theNode->dataCnt();
			if (offset>0) {
				// move some data in right node to the node
				for (int i = theNode->dataCnt();i < totalCnt / 2;i++) {
					theNode->data()[i] = rightNode->data()[i - theNode->dataCnt()];
					theNode->ptrs()[i + 1] = rightNode->ptrs()[i - theNode->dataCnt() + 1];
				}
				// shift right node
				for (int i = offset;i < rightNode->dataCnt();i++) {
					rightNode->data()[i - offset] = rightNode->data()[i];
					rightNode->ptrs()[i - offset + 1] = rightNode->ptrs()[i + 1];
				}
			}
			else {
				offset = -offset;
				// move some data in the node to right node
				for (int i = rightNode->dataCnt()-1;i >= offset;i--) {
					rightNode->data()[i] = rightNode->data()[i - offset];
					rightNode->ptrs()[i + 1] = rightNode->ptrs()[i - offset + 1];
				}
				// shift the node
				for (int i = 0;i < offset;i++) {
					rightNode->data()[i] = theNode->data()[totalCnt / 2 + i];
					rightNode->ptrs()[i + 1] = theNode->ptrs()[totalCnt / 2 + i + 1];
				}
			}
			//update entry in parent node
			int indexInParent = findFirstInBlock(oldKey, parentNode);
			while (parentNode->ptrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
			parentNode->data()[indexInParent] = rightNode->data()[0];
			//update dataCnt
			theNode->dataCnt() = totalCnt / 2;
			rightNode->dataCnt() = totalCnt - totalCnt / 2;
		}
		// put all data in one node
		else {
			// merge data
			for (int i = theNode->dataCnt();i < totalCnt;i++) {
				theNode->data()[i] = rightNode->data()[i - theNode->dataCnt()];
			}
			// delete entry of right node in parent node
			int indexInParent = findFirstInBlock(rightNode->data()[0], parentNode);
			while (parentNode->ptrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
			removeInBlock(parentNode, indexInParent);
			//update dataCnt
			theNode->dataCnt() = totalCnt;
			//delete block
			Block* tmp = rightNode;
			bufferManager->DeleteBlock(tmp);
		}
	}
	//get minimal dataCnt in a node
	int getMinCnt(BPlusNode<T>* theNode) {
		int minCnt;
		if (root == theNode) minCnt = 1;
		else if (theNode->isLeaf()) minCnt = ceil((order - 1)*0.5);
		else minCnt = ceil(order*0.5) - 1;
		return minCnt;
	}
	//operate the whole tree recursively
	void operateTree(BPlusNode<T>* theNode,void (BufferManager::*func)(Block*&) ) {
		if (!theNode->isLeaf()) {
			for (int i = 0;i < theNode->dataCnt() + 1;i++) {
				operateTree(static_cast<BPlusNode<T>*>(bufferManager->GetBlock(theNode->ptrs()[i])),func);
			}
		}
		Block* tmp = theNode;
		(bufferManager->*func)(tmp);//cannot convert type here since reference cannot attach rvalue
	}
};

