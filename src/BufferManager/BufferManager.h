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
	static BufferManager & Instance(){
		//TODO: add a exclusive lock to support multi-thread
		static BufferManager theBufferManager;
		return theBufferManager;
	}
	~BufferManager();

	Block* GetBlock(uint32_t blk_index);
	void ReleaseBlock(uint32_t blk_index);
	void DeleteBlock(uint32_t blk_index);
	uint32_t CreateBlock();
private:
	BufferManager();
	BufferManager(const BufferManager &);
	BufferManager & operator=(const BufferManager &);


	uint32_t AllocNewBlock(); // reserve a block in the db file 
	void WriteBack(BlockNode* blk_node_ptr){
		this->WriteToDisc(blk_node_ptr->data);
		blk_node_ptr->is_modified = false;
	}

	void LoadFromDisc(uint32_t block_index, Block* block_ptr);
	void WriteToDisc(Block* block_ptr);

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
	BlockNode* & GetBlockNode(uint32_t blk_index); // find block node through hash table
	BlockNode* block_table[BLOCK_NUM*2];
};

#endif