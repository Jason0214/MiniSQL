#ifndef _BUFFER_MANAGER_
#define _BUFFER_MANAGER_
#include <string>
#include <cstring>
#include <iostream>
#include "Block.h"

#define DB_FILE ("data.db")
#define BLOCK_NUM (1 << 16) // support buffer size 128MB

class BlockNode{
public:
	BlockNode():pre(NULL),next(NULL),data(NULL),refer_count(0),is_pined(false){}
	~BlockNode(){
		delete data;
	}
	BlockNode* pre;
	BlockNode* next;
	Block* data;
	int refer_count;
	bool is_pined; // if a block is pined, it is being write back, currently not used
};

class BufferManager{
public:
	static BufferManager & Instance(){
		//TODO: add a exclusive lock to support multi-thread
		static BufferManager theBufferManager;
		return theBufferManager;
	}
	~BufferManager() {
		this->RemoveAllBlock();
	};

	Block* GetBlock(uint32_t blk_index);
	Block* CreateBlock(DBenum block_type);
//	const Block* GetBlock(uint32_t blk_index) const;
	void WriteToDisc(Block* block_ptr);
	void ReleaseBlock(Block* block_ptr);
	void DeleteBlock(Block* block_ptr);
	void RemoveAllBlock();
private:
	BufferManager();
	BufferManager(const BufferManager &);
	BufferManager & operator=(const BufferManager &);

	uint32_t AllocNewBlock();  
	void WriteBack(BlockNode* blk_node_ptr){
		this->WriteToDisc(blk_node_ptr->data);
	}

	Block* LoadFromDisc(uint32_t block_index);

	void CreateSrcFile();
	void LoadSrcFile();

	const uint32_t MAX_BLOCK_NUM;
	const std::string SRC_FILE_NAME;

	BlockNode* AddBlock(Block*); 
	void RemoveBlock(BlockNode*);
// list structure for buffer-disc interchange
	BlockNode* block_list_head; 
	BlockNode* block_list_tail;
	uint32_t block_num;

//  table for fast access block
	uint64_t hash(uint32_t blk_index);
	BlockNode* & GetBlockNode(uint32_t blk_index); 
	BlockNode* block_table[BLOCK_NUM*2];
};

#endif