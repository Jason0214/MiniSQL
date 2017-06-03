#ifndef _BLOCK_H_
#define _BLOCK_H_

#define BLOCK_SIZE 4096
#define BLOCK_HEAD_SIZE 10
#include "../CONSTANT.h"
#include <cstdint>
#include <cstring>

// only class Block has real data, 
// other successor only provide addtional method
class Block{
public:
	Block(){}
	virtual ~Block(){}
	virtual void Init(uint32_t index, DBenum block_type=DB_TEMP_BLOCK); //for new block build by upper level 
	uint8_t & BlockType(){
		return *((uint8_t*)&(this->block_data[0]));
	}
	uint8_t & ReservedBytes(){
		return *((uint8_t*)&(this->block_data[1]));
	}
	uint32_t & BlockIndex(){
		return *((uint32_t*)&(this->block_data[2]));
	}
	uint32_t & NextBlockIndex(){
		return *((uint32_t*)&(this->block_data[6]));
	}	
	uint8_t block_data[BLOCK_SIZE];
};

class SchemaBlock:public Block{
public:
	SchemaBlock():Block(){}
	~SchemaBlock() {}
	void Init();
	uint16_t & EmptyPtr(){
		return *((uint16_t*)&(this->block_data[BLOCK_HEAD_SIZE]));
	}
	uint16_t EmptyLen(){
		return BLOCK_SIZE - this->EmptyPtr();
	}
	uint32_t & EmptyBlockAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 2]));
	}
	uint32_t & DBMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 6]));
	}
	uint32_t & UserMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 10]));
	}
	uint32_t & PrivilegeMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 14]));
	}
};

class TableBlock:public Block{
public:
	TableBlock();
	~TableBlock();
	void Init(uint32_t blk_index){
		this->Block::Init(blk_index, DB_TABLE_BLOCK);
		this->RecordNum() = 0;
		this->StackPtr() = BLOCK_SIZE - 1;
	}
	uint16_t & RecordNum(){
		return *((uint16_t*)&(this->block_data[BLOCK_HEAD_SIZE]));
	}
	// 33B table_name 4B table_addr 1B attr_num 2B data offset  
	// 32B attr_name 2B attr_type
	void InsertToHead(const char* table_name, uint32_t table_addr, uint8_t attr_num){
		uint16_t table_offset = DATA_BEG + TABLE_RECORD_SIZE*this->RecordNum();
		strcpy((char*)&(this->block_data[table_offset]), table_name);
		*((uint32_t*)&(this->block_data[table_offset+32])) = table_addr;
		*((uint16_t*)&(this->block_data[table_offset+36])) = this->StackPtr();
		*((uint8_t*)&(this->block_data[table_offset+38])) = attr_num;		
		*((uint32_t*)&(this->block_data[table_offset+39])) = 0;		
		this->RecordNum()++;
	}
	void InsertToTail(const char* attr_name_list[], DBenum* attr_type_list, DBenum* attr_constrain_list, uint8_t attr_num){
		for(uint8_t i = 0; i < attr_num; i++){
			this->StackPtr() -= 32;
			strcpy((char*)&(this->block_data[this->StackPtr()]), attr_name_list[i]);
			this->StackPtr() -= 2;
			*((uint16_t*)&(this->block_data[this->StackPtr()])) = (uint16_t)attr_type_list[i];
			this->StackPtr() -= 2;
			*((uint16_t*)&(this->block_data[this->StackPtr()])) = (uint16_t)attr_constrain_list[i];
			this->StackPtr() -= 4;
			*((uint32_t*)&(this->block_data[this->StackPtr()])) = 0;
		}
	}
	short EmptySize(){
		return this->StackPtr()+1 - DATA_BEG - this->RecordNum() * TABLE_RECORD_SIZE;
	}
	uint16_t & StackPtr(){
		return *((uint16_t*)&(this->block_data[BLOCK_HEAD_SIZE + 2]));
	}
	static const size_t DATA_BEG = BLOCK_HEAD_SIZE + 4;
	static const size_t TABLE_RECORD_SIZE =  43;
	static const size_t ATTR_RECORD_SIZE = 40;
};

class RecordBlock:public Block{
public:
	RecordBlock();
	~RecordBlock();
	void Init();
	uint32_t & RecordNum(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE]));
	}
	uint8_t* GetDataPtr(unsigned int offset){
		return ((uint8_t*)&(this->block_data[BLOCK_HEAD_SIZE+offset]));
	}
private:

};

#endif