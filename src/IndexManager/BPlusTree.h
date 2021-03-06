#pragma once

#include <cmath>
#include <cassert>
#include "../SharedFunc.h"
#include "../BufferManager/Block.h"
#include "IndexExecutor.h"

class BPlusTree : public IndexExecutor {
public:
	BPlusTree(uint32_t root_addr, DBenum key_type) 
		: root(root_addr),
		type(key_type){
		this->SetOrder();
	};
	~BPlusTree() {}

	//insert a entry into the b+tree
	void insert(const void* key, uint32_t addr);


	//search according to the key
	//return a pointer to BPlusTreeSearchResult object
	//the result if the first matched key in the linked list
	//or the smallest larger one
	SearchResult* search(const void* key);
	
	//print out all leaf nodes
	void printAll();

	//get root block pointer
	uint32_t getRoot() {
		return this->root;
	}

	//remove a entry from the b+tree
	//pos must be in the tree or an error will occur
	void remove(SearchResult* pos) {
		removeInBlock(pos->node.raw_ptr, pos->index);
	}

	//remove the whole b+tree
	void removeAll();

	//initialize a block (always the root node)
	void initBlock(uint32_t block_addr);
protected:
	uint32_t root;
	int order;
	DBenum type;
	size_t key_len;

	BPlusNode* BlockToBPNode(Block* block_ptr){
		BPlusNode* ret = dynamic_cast<BPlusNode*>(block_ptr);
		ret->order = this->order;
		ret->key_len = this->key_len;
		return ret;
	}

	//get minimal dataCnt in a node
	int getMinCnt(BPlusNode* theNode) {
		int ret;
		if (this->root == theNode->BlockIndex()) ret = 1;
		else if (theNode->isLeaf()) ret = this->order & 1 ? (this->order - 1) >> 1 : this->order >> 1;
		else ret = this->order & 1 ? this->order >> 1 : (this->order >> 1) - 1;
		return ret;
	}

	void SetOrder(){
		this->key_len = typeLen(this->type);
		this->order = (Block::BLOCK_SIZE - BPlusNode::BPNODE_HEAD_SIZE - Block::BLOCK_HEAD_SIZE) / (this->key_len + sizeof(uint32_t));
		assert(this->order > 0);
	}
	void recursiveDelete(BPlusNode *);

	// insert a new entry
	void insertInBlock(const void* key, uint32_t addr, BPlusNode* theNode);

	//find the smallest larger entry in the block
	int findLargerInBlock(const void* key, BPlusNode* theNode);

	//find the first matched key in the block
	int findFirstInBlock(const void* key, BPlusNode* theNode) {
		int index = findLargerInBlock(key, theNode);
		//trace back the first matched key
		while (index > 0 && compare(theNode->getKey(index - 1), key, this->type) == 0) index--;
		return index;
	}

	//split the current node
	void split(BPlusNode* theNode);

	//delete an entry from the block
	void removeInBlock(BPlusNode* theNode, unsigned int index);

	//merge the current node to other node
	void merge(BPlusNode* theNode);
};