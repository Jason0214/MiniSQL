#ifndef _BLOCK_H_
#define _BLOCK_H_

#define BLOCK_SIZE 4096
#define BLOCK_HEAD_SIZE 12
#include "../CONSTANT.h"
#include <string>

// only class Block has real data, 
// other successor only provide addtional method
class Block{
public:
	Block(){}
	virtual ~Block(){}
	virtual void Init(unsigned int index, DBenum block_type=DB_TEMP_BLOCK); //for new block build by upper level 
	void ReadFromFile(const std::string & ,unsigned int block_index); // for read block from file
	void WriteToDisc(const std::string &);
	unsigned char & BlockType(){
		return *((unsigned char*)&(this->block_data[0]));
	}
	unsigned char & ReservedBytes(){
		return *((unsigned char*)&(this->block_data[1]));
	}
	unsigned int & BlockIndex(){
		return *((unsigned int*)&(this->block_data[2]));
	}
	unsigned int & NextBlockIndex(){
		return *((unsigned int*)&(this->block_data[6]));
	}
	unsigned short & EmptyPtr(){
		return *(unsigned short*)&(this->block_data[10]);
	}
	unsigned short EmptyLen(){
		return BLOCK_SIZE - this->EmptyPtr();		
	}
protected:
	char block_data[BLOCK_SIZE];
};

class SchemaBlock:public Block{
public:
	SchemaBlock():Block(){};
	~SchemaBlock() {};
	void Init();
	unsigned int & EmptyBlockAddr(){
		return *((unsigned int*)&(this->block_data[BLOCK_HEAD_SIZE]));
	}
	unsigned int & DBMetaAddr(){
		return *((unsigned int*)&(this->block_data[BLOCK_HEAD_SIZE+4]));
	}
	unsigned int & UserMetaAddr(){
		return *((unsigned int*)&(this->block_data[BLOCK_HEAD_SIZE+8]));
	}
};

#endif