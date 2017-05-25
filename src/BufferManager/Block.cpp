#include "Block.h"
#include <cstdlib>

using namespace std;

void Block::Init(unsigned int index, DBenum block_type){
	this->BlockType() = (unsigned char)block_type;
	this->ReservedBytes() = 0;
	this->BlockIndex() = index;
	this->NextBlockIndex() = 0;
	this->EmptyPtr() = BLOCK_HEAD_SIZE;
}

void Block::ReadFromFile(const string & file_path, unsigned int blk_index){
	FILE* fp = fopen(file_path.c_str(),"rb");
	//XXX: possible solution: https://stackoverflow.com/questions/6980063/how-to-handle-file-whose-size-is-more-than-2-gb
	fseek(fp, 0, SEEK_SET);
	while(blk_index >= (1 << 19)){
		fseek(fp, (int)((unsigned int)(1 << 31) - 1), SEEK_CUR);
		blk_index -= (1 << 19);
	}
	fseek(fp, blk_index << 12, SEEK_CUR);
	fread(this->block_data, BLOCK_SIZE, 1, fp);
	fclose(fp);
}

void Block::WriteToDisc(const string & file_path){
	unsigned int blk_index = this->BlockIndex();
	FILE* fp = fopen(file_path.c_str(),"rb+");
	//XXX: possible solution: https://stackoverflow.com/questions/6980063/how-to-handle-file-whose-size-is-more-than-2-gb
	fseek(fp, 0, SEEK_SET);
	while(blk_index >= (1 << 19)){
		fseek(fp, (int)((unsigned int)(1 << 31) - 1), SEEK_CUR);
		blk_index -= (1 << 19);
	}
	fseek(fp, blk_index << 12, SEEK_CUR);
	fwrite(this->block_data, BLOCK_SIZE, 1, fp);
	fclose(fp);
}

void SchemaBlock::Init(){
	this->Block::Init(0, DB_SCHEMA_BLOCK);
	this->EmptyBlockAddr() = 0;
	this->UserMetaAddr() = 0;
	this->DBMetaAddr() = 0;
}

