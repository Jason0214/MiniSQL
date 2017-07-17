#include "BufferManager.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cassert>
#include <iostream>
using namespace std;

// make a new SRC_FILE
// or load the existing one
BufferManager::BufferManager():SRC_FILE_NAME(DB_FILE),MAX_BLOCK_NUM(BLOCK_NUM){
	this->block_list_head = NULL;
	this->block_list_tail = NULL;
	this->block_num = 0;
	memset(this->block_table,0,sizeof(BlockNode*)*(BLOCK_NUM<<1));
	if(_access(this->SRC_FILE_NAME.c_str(),0) == -1){
		FILE* fp = fopen(this->SRC_FILE_NAME.c_str(), "w");
		fclose(fp);
		this->CreateSrcFile();
	}
	else{
		this->LoadSrcFile();
	}
	this->pinned_block_count = 0;
}
void BufferManager::CreateSrcFile() {
	SchemaBlock* block_zero = new SchemaBlock();
	block_zero->Init(0,DB_SCHEMA_BLOCK);
	this->WriteToDisc(block_zero);
}

void BufferManager::LoadSrcFile() {
	Block* block_zero = this->LoadFromDisc(0);
}


// before exit, check and write back all the blocks
void BufferManager::RemoveAllBlock() {
	while (this->block_list_head) {
		this->RemoveBlock(this->block_list_head);
	}
}

// hash block index to fit into the hash table
uint64_t BufferManager::hash(uint32_t blk_index){
	// sdmb hash, reference: http://www.cse.yorku.ca/~oz/hash.html
	char* byte_ptr = (char*)&(blk_index);
	unsigned long long hash = 0;
	for(int i = 0; i < sizeof(uint32_t); i++){
		hash = *byte_ptr + (hash << 6) + (hash << 16) - hash;
		byte_ptr++;
	}
	return hash % (BLOCK_NUM << 1);
}

// found a block node when given the block index
BlockNode* & BufferManager::GetBlockNode(uint32_t block_index){
	bool flag = false;
	uint64_t index = this->hash(block_index);
	while(this->block_table[index]){
		if(this->block_table[index]->data->BlockIndex() == block_index) break;
		index++;
		if (index == BLOCK_NUM << 1) index = 0;
	}
	return this->block_table[index];
}

// interface for outer model to get a block
// if block in buffer, move it to the head of the list and return it
// otherwise read it from src file and insert to the head of list and return
Block* BufferManager::GetBlock(uint32_t block_index){
	BlockNode* block_node_ptr = this->GetBlockNode(block_index);
	if(block_node_ptr){
		if(block_node_ptr->pre){
			block_node_ptr->pre->next = block_node_ptr->next;
		}
		else{
			this->block_list_head = block_node_ptr->next;
		}
		if(block_node_ptr->next){
			block_node_ptr->next->pre = block_node_ptr->pre;
		}
		else{
			this->block_list_tail = block_node_ptr->pre;
		}
		block_node_ptr->pre = NULL;
		if(!this->block_list_head){
			block_node_ptr->next=NULL;
			this->block_list_head = block_node_ptr;
			this->block_list_tail = block_node_ptr;
		}
		else{
			block_node_ptr->next = this->block_list_head;
			this->block_list_head->pre = block_node_ptr;
			this->block_list_head = block_node_ptr;
		}
		block_node_ptr->refer_count++;
		return block_node_ptr->data;
	}
	else{
		Block* block_ptr = this->LoadFromDisc(block_index);
		block_node_ptr = this->AddBlock(block_ptr);
		block_node_ptr->refer_count++;	
		return block_ptr;
	}
}

// interface for outer program to return a block
// when outer program get a block, it will be 'pinned'
// which means the block won't be freed by buffer manager
// here release the 'pinned' flag
void BufferManager::ReleaseBlock(Block* block_ptr){
	BlockNode* block_node_ptr = this->GetBlockNode(block_ptr->BlockIndex());
	block_ptr = NULL;
	block_node_ptr->refer_count--;
}

// interface for outer program to get an empty block which not on the disc
Block* BufferManager::CreateBlock(DBenum block_type) {
	Block* block_ptr = NULL;
	switch (block_type) {
	case(DB_TABLE_BLOCK) : block_ptr = new TableBlock(); break;
	case(DB_SCHEMA_BLOCK) : block_ptr = new SchemaBlock(); break;
	case(DB_BPNODE_BLOCK) : block_ptr = new BPlusNode(); break;
	default: block_ptr = new RecordBlock(); break;
	}
	block_ptr->Init(this->AllocNewBlock(),block_type);
	this->WriteToDisc(block_ptr);
	BlockNode* block_node_ptr = this->AddBlock(block_ptr);
	block_node_ptr->refer_count++;
	return block_ptr;
}

// load a block from disc
Block* BufferManager::LoadFromDisc(uint32_t blk_index){
	FILE* fp = fopen(this->SRC_FILE_NAME.c_str(),"rb");
	Block* block_ptr = NULL;
	uint8_t* buf = new uint8_t[Block::BLOCK_SIZE];
	//XXX: possible solution: https://stackoverflow.com/questions/6980063/how-to-handle-file-whose-size-is-more-than-2-gb
	fseek(fp, 0, SEEK_SET);
	while(blk_index >= (1 << 19)){
		fseek(fp, (int)((uint32_t)(1 << 31) - 1), SEEK_CUR);
		blk_index -= (1 << 19);
	}
	fseek(fp, blk_index << 12, SEEK_CUR);
	fread(buf, Block::BLOCK_SIZE, 1, fp);
	switch ((DBenum)*(uint8_t*)buf) {
		case(DB_TABLE_BLOCK) : block_ptr = new TableBlock(buf); break;
		case(DB_SCHEMA_BLOCK) : block_ptr = new SchemaBlock(buf); break;
		case(DB_BPNODE_BLOCK) : block_ptr = new BPlusNode(buf); break;
		case(DB_DELETED_BLOCK) : block_ptr = new Block(buf); break;
		default: block_ptr = new RecordBlock(buf); break;
	}
	fclose(fp);
	return block_ptr;
}

// write a block's data to disc, 
// however it doesn't free the block from the buffer
void BufferManager::WriteToDisc(Block* block_ptr){
	uint32_t blk_index = block_ptr->BlockIndex();
	FILE* fp = fopen(this->SRC_FILE_NAME.c_str(),"rb+");
	//XXX: possible solution: https://stackoverflow.com/questions/6980063/how-to-handle-file-whose-size-is-more-than-2-gb
	fseek(fp, 0, SEEK_SET);
	while(blk_index >= (1 << 19)){
		fseek(fp, (int)((uint32_t)(1 << 31) - 1), SEEK_CUR);
		blk_index -= (1 << 19);
	}
	fseek(fp, blk_index << 12, SEEK_CUR);
	fwrite(block_ptr->block_data, Block::BLOCK_SIZE, 1, fp);
	fclose(fp);	
	block_ptr->is_dirty = false;
}

// add a block to the head of the list
BlockNode* BufferManager::AddBlock(Block* blk_to_add){
	BlockNode* new_node = new BlockNode();
	new_node->data = blk_to_add;

	this->GetBlockNode(blk_to_add->BlockIndex()) = new_node;
	if(this->block_num >= this->MAX_BLOCK_NUM){
		BlockNode* last_node_ptr = this->block_list_tail;
		last_node_ptr->pre->next = NULL;
		this->block_list_tail = last_node_ptr->pre;
		while(last_node_ptr->refer_count > 0){
			last_node_ptr = last_node_ptr->pre;			
		}
		this->RemoveBlock(last_node_ptr);
	}
	if(!this->block_list_head){
		this->block_list_head = new_node;
		this->block_list_tail = new_node;
	}
	else{
		new_node->next = this->block_list_head;
		this->block_list_head->pre = new_node;
		this->block_list_head = new_node;
	}
	this->block_num++;
	return new_node;
}

// free a block from buffer
void BufferManager::RemoveBlock(BlockNode* node_to_remove){
//	if(node_to_remove->data->is_dirty){
//		this->WriteBack(node_to_remove);
//	}
	this->WriteBack(node_to_remove);
	if(node_to_remove->pre){
		node_to_remove->pre->next = node_to_remove->next;
	}
	else{
		this->block_list_head = node_to_remove->next;
	}
	if(node_to_remove->next){
		node_to_remove->next->pre = node_to_remove->pre;
	}
	else{
		this->block_list_tail = node_to_remove->pre;
	}
	this->block_num--;
	// remove from hash table
	this->GetBlockNode(node_to_remove->data->BlockIndex()) = NULL;
	// free memory
	delete node_to_remove;
}

// reserve a new block on the disc and return the block index of the new block
uint32_t BufferManager::AllocNewBlock(){
	SchemaBlock* schema_block = dynamic_cast<SchemaBlock*>(this->GetBlock(0));
	uint32_t deleted_block_addr = schema_block->EmptyBlockAddr();
	if(deleted_block_addr == 0){
		// no empty block in db file, have to expand the db file
		uint64_t size;
		struct __stat64 st;
		_stat64(this->SRC_FILE_NAME.c_str(), &st);
		size = st.st_size;
		// reference: https://stackoverflow.com/questions/2361385/how-to-get-a-files-size-which-is-greater-than-4-gb
		this->ReleaseBlock((Block* &)schema_block);
		return (uint32_t)(size >> 12);
	}
	else{
		// empty block in db file, allocate this block
		Block* deleted_block = this->GetBlock(deleted_block_addr);
		schema_block->EmptyBlockAddr() = deleted_block->NextBlockIndex();
		// schema being changed
		schema_block->is_dirty = true;
		this->ReleaseBlock((Block* &)schema_block);
		this->RemoveBlock(this->GetBlockNode(deleted_block->BlockIndex()));
		return deleted_block_addr;
	}
}

// interface for outer program to delete a block currently on the disc
void BufferManager::DeleteBlock(Block* block_ptr){
	/* do not forget to set the next field of the previous block */
	BlockNode* block_node_ptr = this->GetBlockNode(block_ptr->BlockIndex());
	block_ptr = NULL;
	if(block_node_ptr){
		Block* block_to_delete = block_node_ptr->data;
		
		// update schema	
		SchemaBlock* schema_block = dynamic_cast<SchemaBlock*>(this->GetBlock(0));

		block_to_delete->NextBlockIndex() = schema_block->EmptyBlockAddr();
		block_to_delete->BlockType() = (uint8_t)DB_DELETED_BLOCK;
		
		schema_block->EmptyBlockAddr() = block_to_delete->BlockIndex();
		schema_block->is_dirty = true;
		this->ReleaseBlock((Block* &)schema_block);

		block_to_delete->is_dirty = true;
//		cout << block_node_ptr->refer_count << endl;
		assert(--block_node_ptr->refer_count == 0);
	}
}