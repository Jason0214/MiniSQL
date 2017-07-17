#include "BPlusTree.h"
#include "../BufferManager/BufferManager.h"

#define INDEX_MANAGER_DEBUG

static BufferManager & buffer_manager = BufferManager::Instance();

void BPlusTree::insert(const void* key, uint32_t addr){
	BPlusNode* theNode;
	//normal case
	theNode = this->root;
	if(!this->root->isLeaf()){
		theNode = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[findFirstInBlock(key, theNode)]));
		while (!theNode->isLeaf()) {
			BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[findLargerInBlock(key, theNode)]));
			buffer_manager.ReleaseBlock(theNode);
			theNode = next;
		}
	}
	insertInBlock(key, addr, theNode);
	theNode->is_dirty = true;
	buffer_manager.ReleaseBlock(theNode);
}

SearchResult* BPlusTree::search(const void* key) {
	BPlusNode* theNode = this->root;
	//retunr null pointer if the tree is empty
	if (!this->root->dataCnt()) {
		return nullptr;
	}
	// can not release root in the tree
	if(!this->root->isLeaf()){
		theNode = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[findFirstInBlock(key, theNode)]));
		while (!theNode->isLeaf()) {
			BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[findFirstInBlock(key, theNode)]));
			buffer_manager.ReleaseBlock(theNode);
			theNode = next;
		}
	}
	SearchResult* result = new SearchResult();
	result->index = findFirstInBlock(key, theNode);
	result->node = theNode;
	return result;
}

// insert a new entry
void BPlusTree::insertInBlock(const void* key, uint32_t addr, BPlusNode* theNode) {
	//std::cout << '<'<<theNode->dataCnt() << '>';
	int idx = this->findLargerInBlock(key, theNode);
	for (int i = theNode->dataCnt() - 1;i >= idx;i--) {
		memcpy(theNode->getKey(i + 1), theNode->getKey(i), this->key_len);
		theNode->addrs()[i + 2] = theNode->addrs()[i + 1];
	}
	memcpy(theNode->getKey(idx), key, this->key_len);
	theNode->addrs()[idx + 1] = addr;

	theNode->dataCnt()++;
	//if needed, split the node
	if (theNode->dataCnt() >= this->order) {
		split(theNode);
	}
}

int BPlusTree::findLargerInBlock(const void* key, BPlusNode* theNode) {
	int low = 0, mid, high = theNode->dataCnt() - 1;
	// binary search
	while (low <= high)
	{
		mid = (low + high) >> 1;
		int cmp_result = compare(theNode->getKey(mid), key, this->type);
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
	if (theNode == this->root) {
		//create a new root
		this->root = BlockToBPNode(buffer_manager.CreateBlock(DB_BPNODE_BLOCK));
		this->root->isLeaf() = false;
		this->root->dataCnt() = 0;
		this->root->addrs()[0] = theNode->BlockIndex();
		this->root->parent() = 0;
		this->root->rightSibling() = 0;
		theNode->parent() = this->root->BlockIndex();
	}
	//leaf node
	if (theNode->isLeaf()) {
		//copy data to new node
		memcpy(newNode->getKey(0), theNode->getKey(this->order - (this->order >> 1)),
													(this->order >> 1) * this->key_len);
		memcpy(&(newNode->addrs()[1]), &(theNode->addrs()[this->order - (this->order >> 1) + 1]), 
													(this->order >> 1) * this->key_len);
		newNode->isLeaf() = true;
		newNode->parent() = theNode->parent();
		newNode->dataCnt() = this->order >> 1;
		theNode->dataCnt() = this->order - (this->order >> 1);
		//insert new entry to parent
		BPlusNode* parent_node = BlockToBPNode(buffer_manager.GetBlock(newNode->parent()));
		insertInBlock(newNode->getKey(0), newNode->BlockIndex(), parent_node);
		parent_node->is_dirty = true;
		buffer_manager.ReleaseBlock(parent_node);
	}
	//non-leaf node
	else {
		//copy data to new node
		memcpy(newNode->getKey(0), theNode->getKey((this->order >> 1) + 1),
										(this->order - (this->order >> 1) - 1) * this->key_len);
		memcpy(&(newNode->addrs()[0]), &(theNode->addrs()[(this->order >> 1) + 1]), 
										(this->order - (this->order >> 1)) * this->key_len);
		newNode->isLeaf() = false;
		newNode->parent() = theNode->parent();
		newNode->dataCnt() = this->order - (this->order >> 1) - 1;
		theNode->dataCnt() = this->order >> 1;
		//insert new entry to parent
		BPlusNode* parent_node = BlockToBPNode(buffer_manager.GetBlock(newNode->parent()));
		insertInBlock(theNode->getKey(order >> 1), newNode->BlockIndex(), parent_node);
		parent_node->is_dirty = true;
		buffer_manager.ReleaseBlock(parent_node);
	}
	theNode->is_dirty = true;
	newNode->is_dirty = true;
	buffer_manager.ReleaseBlock(newNode);
}

void BPlusTree::removeInBlock(BPlusNode* & theNode, unsigned int index) {
	for (int i = index; i < theNode->dataCnt() - 1; i++) {
		memcpy(theNode->getKey(i), theNode->getKey(i + 1), this->key_len);
		theNode->addrs()[i + 1] = theNode->addrs()[i + 2];
	}
	theNode->dataCnt()--;
	//check if there's too few data
	int minCnt = getMinCnt(theNode);
	theNode->is_dirty = true;
	if (!theNode->dataCnt() && theNode == this->root && theNode->isLeaf()) {
		return;
	}
	else if (theNode->dataCnt() < minCnt) {
		merge(theNode);
	}
}

void BPlusTree::merge(BPlusNode* & theNode) {
	BPlusNode* rightNode;
	//if the node is the root, let its only child to be the new root
	if (theNode == this->root) {
		Block* oldRoot = this->root;
		//if root is a leaf, do nothing
		//if (theNode->isLeaf()) root = nullptr;
		if (theNode->isLeaf());
		//assign new root
		else {
			this->root = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[0]));
			buffer_manager.DeleteBlock(oldRoot);
		}
		return;
	}
	BPlusNode* parentNode = BlockToBPNode(buffer_manager.GetBlock(theNode->parent()));
	//if the node is the last child of parent node, assign the node to be its left sibling
	if (parentNode->addrs()[parentNode->dataCnt()] == theNode->BlockIndex()) {
		rightNode = theNode;
		theNode = BlockToBPNode(buffer_manager.GetBlock(parentNode->addrs()[parentNode->dataCnt() - 1]));
	}
	//normally right node is the right sibling of the node
	else {
		rightNode = BlockToBPNode(buffer_manager.GetBlock(theNode->rightSibling()));
	}
	int totalCnt = theNode->dataCnt() + rightNode->dataCnt();
	//cannot put in one node, redistribute entries
	if (totalCnt > this->order - 1) {
		const void* oldKey = rightNode->getKey(0);
		int offset = (totalCnt >> 1) - theNode->dataCnt();
		if (offset > 0) {
			// move some data in right node to the node
			for (int i = theNode->dataCnt();i < (totalCnt >> 1);i++) {
				memcpy(theNode->getKey(i), rightNode->getKey(i - theNode->dataCnt()), this->key_len);
				theNode->addrs()[i + 1] = rightNode->addrs()[i - theNode->dataCnt() + 1];
			}
			// shift right node
			for (int i = offset;i < rightNode->dataCnt();i++) {
				memcpy(rightNode->getKey(i - offset), rightNode->getKey(i), this->key_len);
				rightNode->addrs()[i - offset + 1] = rightNode->addrs()[i + 1];
			}
		}
		else {
			offset = -offset;
			// move some data in the node to right node
			for (int i = rightNode->dataCnt()-1;i >= offset;i--) {
				memcpy(rightNode->getKey(i), rightNode->getKey(i - offset), this->key_len);
				rightNode->addrs()[i + 1] = rightNode->addrs()[i - offset + 1];
			}
			// shift the node
			for (int i = 0;i < offset;i++) {
				memcpy(rightNode->getKey(i), theNode->getKey((totalCnt >> 1) + i), this->key_len);
				rightNode->addrs()[i + 1] = theNode->addrs()[(totalCnt >> 1) + i + 1];
			}
		}
		//update entry in parent node
		int indexInParent = findFirstInBlock(oldKey, parentNode);
		while (parentNode->addrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
		memcpy(parentNode->getKey(indexInParent), rightNode->getKey(0), this->key_len);
		//update dataCnt
		theNode->dataCnt() = totalCnt >> 1;
		rightNode->dataCnt() = totalCnt - (totalCnt >> 1);
		theNode->is_dirty = true;
		rightNode->is_dirty = true;
		buffer_manager.ReleaseBlock(rightNode);
	}
	// put all data in one node
	else {
		// merge data
		for (int i = theNode->dataCnt();i < totalCnt;i++) {
			memcpy(theNode->getKey(i), rightNode->getKey(i - theNode->dataCnt()), this->key_len);
		}
		// delete entry of right node in parent node
		int indexInParent = findFirstInBlock(rightNode->getKey(0), parentNode);
		while (parentNode->addrs()[indexInParent + 1] != rightNode->BlockIndex()) indexInParent++;
		removeInBlock(parentNode, indexInParent);
		//update dataCnt
		theNode->dataCnt() = totalCnt;
		//update rightSibling
		theNode->rightSibling() = rightNode->rightSibling();
		//delete block
		theNode->is_dirty = true;
		rightNode->is_dirty = true;
		buffer_manager.DeleteBlock(rightNode);
	}
	parentNode->is_dirty = true;
	buffer_manager.ReleaseBlock(parentNode);
}


//operate the whole tree recursively
void BPlusTree::recursiveDelete(BPlusNode* theNode) {
	if (!theNode->isLeaf()) {
		for (int i = 0;i < theNode->dataCnt() + 1;i++) {
			recursiveDelete(BlockToBPNode((buffer_manager.GetBlock(theNode->addrs()[i]))));
		}
	}
	buffer_manager.DeleteBlock(theNode);
}

//remove the whole b+tree
void BPlusTree::removeAll() {
	BPlusNode* theNode = this->root;
	if (!theNode->isLeaf()) {
		for (int i = 0; i < theNode->dataCnt() + 1; i++) {
			recursiveDelete(BlockToBPNode((buffer_manager.GetBlock(theNode->addrs()[i]))));
		}
	}
	theNode->dataCnt() = 0;
	theNode->is_dirty = true;
}

#ifdef INDEX_MANAGER_DEBUG
//print out all leaf nodes
void BPlusTree::printAll() {
	BPlusNode* theNode = this->root;
	while (!theNode->isLeaf()) {
		BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->addrs()[0]));
		buffer_manager.ReleaseBlock(theNode);
		theNode = next;
	}
	while (true) {
		for (int i = 0;i < theNode->dataCnt();i++) {
			printByType(theNode->getKey(i), this->type);
		}
		if (theNode->rightSibling()) {
			BPlusNode* next = BlockToBPNode(buffer_manager.GetBlock(theNode->rightSibling()));
			buffer_manager.ReleaseBlock(theNode);
			theNode = next;
		}
		else {
			break;
		}
	}
}
#endif