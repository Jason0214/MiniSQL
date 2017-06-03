#include "Block.h"
#include <cstdlib>

using namespace std;

void Block::Init(uint32_t index, DBenum block_type){
	this->BlockType() = (uint8_t)block_type;
	this->ReservedBytes() = 0;
	this->BlockIndex() = index;
	this->NextBlockIndex() = 0;
}

void SchemaBlock::Init(){
	this->Block::Init(0, DB_SCHEMA_BLOCK);
	this->EmptyPtr() = BLOCK_HEAD_SIZE;
	this->EmptyBlockAddr() = 0;
	this->UserMetaAddr() = 0;
	this->DBMetaAddr() = 0;
}

