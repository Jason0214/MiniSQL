#include "BPlusTree.h"
#include "../BufferManager/BufferManager.h"

#define INDEX_MANAGER_DEBUG

#ifdef INDEX_MANAGER_DEBUG
	#include <iostream>
#endif

static BufferManager & buffer_manager = BufferManager::Instance();

void BPlusTree::insert(const void* key, uint32_t addr){
	BPlusNode* theNode;
	//normal case
	theNode = this->root;
	while (!theNode->isLeaf()) {
		BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->ptrs()[findLargerInBlock(key, theNode)]));
		buffer_manager.ReleaseBlock((Block* &)theNode);
		theNode = next;
	}
	insertInBlock(key, addr, theNode);
}

SearchResult* BPlusTree::search(const void* key) {
	BPlusNode* theNode = this->root;
	int index;
	//retunr null pointer if the tree is empty
	if (!root->dataCnt()) {
		return NULL;
	}
	while (!theNode->isLeaf()) {
		index = findFirstInBlock(key, theNode);
		BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->ptrs()[index]));
		buffer_manager.ReleaseBlock((Block* &)theNode);
		theNode = next;
	}
	index = findFirstInBlock(key, theNode);
	SearchResult* result = new SearchResult();
	result->index = index;
	result->leaf_node = theNode;
	return result;
}

// insert a new entry
void BPlusTree::insertInBlock(const void* key, uint32_t addr, BPlusNode* theNode) {
	//std::cout << '<'<<theNode->dataCnt() << '>';
	int idx = this->findLargerInBlock(key, theNode);
	for (int i = theNode->dataCnt() - 1;i >= idx;i--) {
		memcpy(theNode->data()[i + 1], theNode->data()[i], this->key_len);
		theNode->ptrs()[i + 2] = theNode->ptrs()[i + 1];
	}
	memcpy(theNode->data()[idx], key, this->key_len);
	theNode->ptrs()[idx + 1] = addr;

	theNode->dataCnt()++;
	//if needed, split the node
	if (theNode->dataCnt() >= this->order) {
		split(theNode);
	}
	buffer_manager.ReleaseBlock((Block* &)theNode);
}

int BPlusTree::findLargerInBlock(const void* key, BPlusNode* theNode) {
	int low = 0, mid, high = theNode->dataCnt() - 1;
	// binary search
	while (low <= high)
	{
		mid = (low + high) >> 1;
		int cmp_result = compare(theNode->data()[mid], key, this->type);
		if (cmp_result < 0) low = mid + 1;
		else if (cmp_result > 0) high = mid - 1;
		else return mid;
	}

	// return low if key is not found
	return low;
}

//split the current node
void BPlusTree::split(BPlusNode* theNode) {
	BPlusNode* newNode = BlockToBPNode(buffer_manager.CreateBlock(DB_BPNODE_BLOCK));
	//maintain linked list
	newNode->rightSibling() = theNode->rightSibling();
	theNode->rightSibling() = newNode->BlockIndex();
	//if the node is the root
	if (theNode == root) {
		//create a new root
		root = BlockToBPNode(buffer_manager.CreateBlock(DB_BPNODE_BLOCK));
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
		memcpy(newNode->data()[0], theNode->data()[order - (order >> 1)], (order >> 1) * this->key_len);
		memcpy(&(newNode->ptrs()[1]), &(theNode->ptrs()[order - (order >> 1) + 1]), (order >> 1) * this->key_len);
		newNode->isLeaf() = true;
		newNode->parent() = theNode->parent();
		newNode->dataCnt() = order >> 1;
		theNode->dataCnt() = order - (order >> 1);
		//insert new entry to parent
		BPlusNode parent_node = BlockToBPNode(buffer_manager.GetBlock(newNode->parent()))
		insertInBlock(newNode->data()[0], newNode->BlockIndex(), parent_node);
		buffer_manager.ReleaseBlock((Block* &)parent_node);
	}
	//non-leaf node
	else {
		//copy data to new node
		memcpy(newNode->data()[0], theNode->data()[(order >> 1) + 1], (order - (order >> 1) - 1) * this->key_len);
		memcpy(&(newNode->ptrs()[0]), &(theNode->ptrs()[(order >> 1) + 1]), (order - (order >> 1)) * this->key_len);
		newNode->isLeaf() = false;
		newNode->parent() = theNode->parent();
		newNode->dataCnt() = order - (order >> 1) - 1;
		theNode->dataCnt() = order >> 1;
		//insert new entry to parent
		BPlusNode parent_node = BlockToBPNode(buffer_manager.GetBlock(newNode->parent()))
		insertInBlock(theNode->data()[order >> 1], newNode->BlockIndex(), parent_node);
		buffer_manager.ReleaseBlock((Block* &)parent_node);
	}
	buffer_manager.ReleaseBlock((Block* &)newNode);
}

void BPlusTree::removeInBlock(BPlusNode* theNode, unsigned int index) {
	for (int i = index;i < theNode->dataCnt() - 1;i++) {
		memcpy(theNode->data()[i], theNode->data()[i + 1], this->key_len);
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
		buffer_manager.ReleaseBlock((Block* &)theNode);
	}
}

void BPlusTree::merge(BPlusNode* & theNode) {
	BPlusNode* rightNode;
	//if the node is the root, let its only child to be the new root
	if (theNode == root) {
		Block* oldRoot = this->root;
		//if root is a leaf, do nothing
		//if (theNode->isLeaf()) root = nullptr;
		if (theNode->isLeaf());
		//assign new root
		else {
			this->root = BlockToBPNode(buffer_manager.GetBlock(theNode->ptrs()[0]));
			buffer_manager.DeleteBlock(oldRoot);
		}
		return;
	}
	BPlusNode* parentNode = BlockToBPNode(buffer_manager.GetBlock(theNode->parent()));
	//if the node is the last child of parent node, assign the node to be its left sibling
	if (parentNode->ptrs()[parentNode->dataCnt()] == theNode->BlockIndex()) {
		rightNode = theNode;
		theNode = BlockToBPNode(buffer_manager.GetBlock(parentNode->ptrs()[parentNode->dataCnt() - 1]));
	}
	//normally right node is the right sibling of the node
	else {
		rightNode = BlockToBPNode(buffer_manager.GetBlock(theNode->rightSibling()));
	}
	int totalCnt = theNode->dataCnt() + rightNode->dataCnt();
	//cannot put in one node, redistribute entries
	if (totalCnt > this->order - 1) {
		const void* oldKey = rightNode->data()[0];
		int offset = (totalCnt >> 1) - theNode->dataCnt();
		if (offset > 0) {
			// move some data in right node to the node
			for (int i = theNode->dataCnt();i < (totalCnt >> 1);i++) {
				memcpy(theNode->data()[i], rightNode->data()[i - theNode->dataCnt()], this->key_len);
				theNode->ptrs()[i + 1] = rightNode->ptrs()[i - theNode->dataCnt() + 1];
			}
			// shift right node
			for (int i = offset;i < rightNode->dataCnt();i++) {
				memcpy(rightNode->data()[i - offset], rightNode->data()[i], this->key_len);
				rightNode->ptrs()[i - offset + 1] = rightNode->ptrs()[i + 1];
			}
		}
		else {
			offset = -offset;
			// move some data in the node to right node
			for (int i = rightNode->dataCnt()-1;i >= offset;i--) {
				memcpy(rightNode->data()[i], rightNode->data()[i - offset], this->key_len);
				rightNode->ptrs()[i + 1] = rightNode->ptrs()[i - offset + 1];
			}
			// shift the node
			for (int i = 0;i < offset;i++) {
				memcpy(rightNode->data()[i], theNode->data()[(totalCnt >> 1) + i], this->key_len);
				rightNode->ptrs()[i + 1] = theNode->ptrs()[(totalCnt >> 1) + i + 1];
			}
		}
		//update entry in parent node
		int indexInParent = findFirstInBlock(oldKey, parentNode);
		while (parentNode->ptrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
		memcpy(parentNode->data()[indexInParent], rightNode->data()[0], this->key_len);
		//update dataCnt
		theNode->dataCnt() = totalCnt >> 1;
		rightNode->dataCnt() = totalCnt - (totalCnt >> 1);
		buffer_manager.ReleaseBlock((Block* &)rightNode);
	}
	// put all data in one node
	else {
		// merge data
		for (int i = theNode->dataCnt();i < totalCnt;i++) {
			memcpy(theNode->data()[i], rightNode->data()[i - theNode->dataCnt()], this->key_len);
		}
		// delete entry of right node in parent node
		int indexInParent = findFirstInBlock(rightNode->data()[0], parentNode);
		while (parentNode->ptrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
		removeInBlock(parentNode, indexInParent);
		//update dataCnt
		theNode->dataCnt() = totalCnt;
		//update rightSibling
		theNode->rightSibling() = rightNode->rightSibling();
		//delete block
		buffer_manager.DeleteBlock((Block* &)rightNode);
	}
}


//operate the whole tree recursively
static void recursiveDelete(BPlusNode* theNode) {
	if (!theNode->isLeaf()) {
		for (int i = 0;i < theNode->dataCnt() + 1;i++) {
			recursiveDelete(buffer_manager->GetBlock(theNode->ptrs()[i]));
		}
	}
	buffer_manager.DeleteBlock((Block* &)theNode);
}

void BPlusTree::removeAll() {
	BPlusNode* theNode = this->root;
	if (!theNode->isLeaf()) {
		for (int i = 0;i < theNode->dataCnt() + 1;i++) {
			recursiveDelete(BlockToBPNode(buffer_manager->GetBlock(theNode->ptrs()[i])));
		}
	}
	theNode->dataCnt() = 0;
}

#ifdef INDEX_MANAGER_DEBUG
//print out all leaf nodes
void BPlusTree::printAll() {
	BPlusNode* theNode = this->root;
	while (!theNode->isLeaf()) {
		next = BlockToBPNode(buffer_manager.GetBlock(theNode->ptrs()[0]));
		buffer_manager.ReleaseBlock((Block* &)theNode);
		theNode = next;
	}
	while (true) {
		for (int i = 0;i < theNode->dataCnt();i++) {
			std::cout << theNode->data()[i] << " ";
		}
		if (theNode->rightSibling()) {
			BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->rightSibling()));
			buffer_manager.ReleaseBlock((Block* &)theNode);
			theNode = next;
		}
		else {
			break;
		}
	}
	std::cout << std::endl;
}
#endif