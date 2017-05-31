#ifndef _BLOCK_H_
#define _BLOCK_H_

#define BLOCK_SIZE 4096
#define BLOCK_HEAD_SIZE 12
#include "../CONSTANT.h"
#include <string>

class BufferManager;

// only class Block has real data, 
// other successor only provide addtional method
class Block{
public:
	Block(){}
	virtual ~Block(){}
	virtual void Init(uint32_t index, DBenum block_type=DB_TEMP_BLOCK); //for new block build by upper level 
	friend class BufferManager;
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
	uint16_t & EmptyPtr(){
		return *((uint16_t*)&(this->block_data[10]));
	}
	uint16_t EmptyLen(){
		return BLOCK_SIZE - this->EmptyPtr();		
	}
protected:
	uint8_t block_data[BLOCK_SIZE];
};

class SchemaBlock:public Block{
public:
	SchemaBlock():Block(){}
	~SchemaBlock() {}
	void Init();
	uint32_t & EmptyBlockAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE]));
	}
	uint32_t & DBMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 4]));
	}
	uint32_t & UserMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 8]));
	}
	uint32_t & PrivilegeMetaAddr(){
		return *((uint32_t*)&(this->block_data[BLOCK_HEAD_SIZE + 12]));
	}
};

#endif