#include "BufferManager.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

BufferManager::BufferManager():SRC_FILE_NAME(DB_FILE),MAX_BLOCK_NUM(BLOCK_NUM){
	this->block_list_head = NULL;
	this->block_list_tail = NULL;
	this->block_num = 0;
	memset(this->block_table,0,sizeof(BlockNode*)*(BLOCK_NUM<<1));
	if(_access(this->SRC_FILE_NAME.c_str(),0) == -1){
		// db file not exist
		FILE* fp = fopen(this->SRC_FILE_NAME.c_str(), "w");
		fclose(fp);
		this->CreateSrcFile();
	}
	else{
		// load db file
		this->LoadSrcFile();
	}
}

BufferManager::~BufferManager() {
	while (this->block_list_head) {
		this->RemoveBlock(this->block_list_head);
	}
}

unsigned long long int BufferManager::hash(unsigned int blk_index){
	// sdmb hash, reference: http://www.cse.yorku.ca/~oz/hash.html
	char* byte_ptr = (char*)&(blk_index);
	unsigned long long hash = 0;
	for(int i = 0; i < sizeof(unsigned int); i++){
		hash = *byte_ptr + (hash << 6) + (hash << 16) - hash;
		byte_ptr++;
	}
	return hash%(BLOCK_NUM << 1);
}

BlockNode* & BufferManager::GetBlockNode(unsigned int block_index){
	bool flag = false;
	unsigned long long int index = this->hash(block_index);
	while(this->block_table[index]){
		if(this->block_table[index]->data->BlockIndex() == block_index) break;
		index++;
	}
	return this->block_table[index];
}

void BufferManager::CreateSrcFile(){
	SchemaBlock* block_zero = new SchemaBlock();
	block_zero->Init();
	this->AddBlock(block_zero, true);
	block_zero->WriteToDisc(this->SRC_FILE_NAME);

	Block* usr_block = new Block();
	usr_block->Init(this->AllocNewBlock(),DB_USER_BLOCK);
	// point to the table of usr block
	block_zero->UserMetaAddr() = usr_block->BlockIndex();
	usr_block->WriteToDisc(this->SRC_FILE_NAME);
	this->AddBlock(usr_block);

	Block* db_block = new Block();
	db_block->Init(this->AllocNewBlock(),DB_DATABASE_BLOCK);
	// point to the table of databases info
	block_zero->DBMetaAddr() = db_block->BlockIndex();
	db_block->WriteToDisc(this->SRC_FILE_NAME);
	this->AddBlock(db_block);

	block_zero->EmptyPtr() = block_zero->EmptyPtr() + 12;
	block_zero->WriteToDisc(this->SRC_FILE_NAME);
}

Block* BufferManager::GetBlock(unsigned int block_index){
	BlockNode* block_node_ptr = this->GetBlockNode(block_index);
	if(block_node_ptr){
		// move to the head of list
		// remove
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
		//add back
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
		return block_node_ptr->data;
	}
	else{
		Block* block_ptr = new Block();
		block_ptr->ReadFromFile(this->SRC_FILE_NAME, block_index);
		this->AddBlock(block_ptr);
		return block_ptr;
	}
}

void BufferManager::LoadSrcFile(){
	SchemaBlock* block_zero = new SchemaBlock();
	block_zero->ReadFromFile(this->SRC_FILE_NAME, 0);
	this->AddBlock(block_zero, true);

	Block* usr_block = new Block();
	usr_block->ReadFromFile(this->SRC_FILE_NAME, block_zero->UserMetaAddr());
	this->AddBlock(usr_block);

	Block* db_block = new Block();
	db_block->ReadFromFile(this->SRC_FILE_NAME, block_zero->DBMetaAddr());
	this->AddBlock(db_block);
}

void BufferManager::AddBlock(Block* blk_to_add, bool to_pin){
	BlockNode* new_node = new BlockNode();
	new_node->data = blk_to_add;
	new_node->is_pined = to_pin;

	this->GetBlockNode(blk_to_add->BlockIndex()) = new_node;
	if(this->block_num >= this->MAX_BLOCK_NUM){
		BlockNode* last_node_ptr = this->block_list_tail;
		last_node_ptr->pre->next = NULL;
		this->block_list_tail = last_node_ptr->pre;
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
}

void BufferManager::RemoveBlock(BlockNode* node_to_remove){
	if(node_to_remove->is_modified){
		this->WriteBack(node_to_remove);
	}
	// remove from list
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
	// remote from hash table
	this->GetBlockNode(node_to_remove->data->BlockIndex()) = NULL;
	// free memory
	delete node_to_remove;
}

unsigned int BufferManager::AllocNewBlock(){
	BlockNode* schema_node = this->block_table[0]; // smdb always hash 0 -> 0
	SchemaBlock* schema_block = dynamic_cast<SchemaBlock*>(schema_node->data);
	unsigned int empty_block_addr = schema_block->EmptyBlockAddr();
	if(empty_block_addr == 0){
		// no empty block in db file, have to expand the db file
		unsigned long long int size;
		struct __stat64 st;
		_stat64(this->SRC_FILE_NAME.c_str(), &st);
		size = st.st_size;
		// reference: https://stackoverflow.com/questions/2361385/how-to-get-a-files-size-which-is-greater-than-4-gb
		return (unsigned int)(size >> 12);
	}
	else{
		// empty block in db file, allocate this block
		schema_block->EmptyPtr() = this->GetBlock(empty_block_addr)->NextBlockIndex();
		// schema being changed
		schema_node->is_modified = true;
		return empty_block_addr;
	}
}

void BufferManager::DeleteFromDisc(Block* block_to_delete){
	BlockNode* schema_node = this->block_table[0]; // smdb always hash 0 -> 0
	SchemaBlock* schema_block = dynamic_cast<SchemaBlock*>(schema_node->data);
	/* do not forget to set the next field of the previous block */
	block_to_delete->BlockType() = (unsigned char)DB_DELETED_BLOCK;
	block_to_delete->NextBlockIndex() = schema_block->EmptyBlockAddr();
	// update schema
	schema_block->EmptyBlockAddr() = block_to_delete->BlockIndex();
	schema_node->is_modified = true;
	BlockNode* block_node_ptr = this->GetBlockNode(block_to_delete->BlockIndex());	
	block_node_ptr->is_modified = true;
	if(block_node_ptr){
		this->RemoveBlock(block_node_ptr);
	}
}