#ifndef _BUFFER_MANAGER_
#define _BUFFER_MANAGER_
#include <string>
#include <cstring>
#include "Block.h"

#define DB_FILE ("data.db")
#define BLOCK_NUM (1 << 16) // support buffer size 128MB

class BlockNode{
public:
	BlockNode():pre(NULL),next(NULL),is_pined(false),data(NULL),is_modified(false){}
	~BlockNode(){
		delete data;
	}
	BlockNode* pre;
	BlockNode* next;
	Block* data;
	bool is_pined; // if a block is pined, it cannot be remove from buffer
	bool is_modified;
};

class BufferManager{
public:
	BufferManager();
	~BufferManager();
	Block* GetBlock(unsigned int blk_index);
	Block* CreateBlock() {
		Block* block_ptr = new Block();
		block_ptr->Init(this->AllocNewBlock());
		this->AddBlock(block_ptr);
		return block_ptr;
	}
	void DeleteFromDisc(Block*);
private:
	unsigned int AllocNewBlock(); // reserve a block in the db file 
	void WriteBack(BlockNode* blk_node_ptr){
		blk_node_ptr->data->WriteToDisc(this->SRC_FILE_NAME);
		blk_node_ptr->is_modified = false;
	}

	void CreateSrcFile();
	void LoadSrcFile();

	const unsigned int MAX_BLOCK_NUM;
	const std::string SRC_FILE_NAME;

	void AddBlock(Block*, bool to_pin=false); // to a block buffer
	void RemoveBlock(BlockNode*);
// list structure for buffer-disc interchange
	BlockNode* block_list_head; 
	BlockNode* block_list_tail;
	unsigned int block_num;

//  table for fast access block
	unsigned long long int hash(unsigned int blk_index);
	BlockNode* & GetBlockNode(unsigned int blk_index); // find block node through hash table
	BlockNode* block_table[BLOCK_NUM*2];
};

#endif