#include "Block.h"
#include <cstdlib>
#include <string>

using namespace std;

void Block::Init(uint32_t index, DBenum block_type){
	this->BlockType() = (uint8_t)block_type;
	this->ReservedBytes() = 0;
	this->BlockIndex() = index;
	this->NextBlockIndex() = 0;
	this->PreBlockIndex() = 0;
	this->is_dirty = false;
}

// Insert a table's meta data order by table_name
// if duplicated name found, raise exception
void TableBlock::InsertTable(const char* table_name, uint32_t table_addr, uint32_t index_addr,uint8_t attr_num, uint8_t key_index){
	int table_index = this->FindRecordIndex(table_name);
	if (table_index < 0) table_index = 0;
	uint16_t target_addr = DATA_BEG + TABLE_RECORD_SIZE * table_index;
	if(this->RecordNum() != 0 && table_index < this->RecordNum() && !strcmp(table_name, (char*)&this->block_data[target_addr])){
		throw DuplicatedTableName(string(table_name));
	}
	uint16_t tail_addr = this->RecordNum() * TABLE_RECORD_SIZE + DATA_BEG;
	while(tail_addr != target_addr){
		memcpy(&this->block_data[tail_addr], &this->block_data[tail_addr-TABLE_RECORD_SIZE], TABLE_RECORD_SIZE);
		tail_addr -= TABLE_RECORD_SIZE;
	}
	strcpy((char*)&(this->block_data[target_addr]), table_name);
	*(uint16_t*)&(this->block_data[target_addr+32]) = this->StackPtr();
	*(uint8_t*)&(this->block_data[target_addr+34]) = attr_num;
	*(uint8_t*)&(this->block_data[target_addr+35]) = key_index;
	*(uint32_t*)&(this->block_data[target_addr+36]) = table_addr;
	*(uint32_t*)&(this->block_data[target_addr+40]) = index_addr;
	this->RecordNum()++;
}

// given an attribute name and an attribute type
// insert all of them into the block
void TableBlock::InsertAttr(const char* attr_name, DBenum attr_type){
		this->StackPtr() -= 32;
		strcpy((char*)&(this->block_data[this->StackPtr()]), attr_name);
		this->StackPtr() -= 2;
		*((uint16_t*)&(this->block_data[this->StackPtr()])) = (uint16_t)attr_type;
		this->StackPtr() -= 2;
		*((uint16_t*)&(this->block_data[this->StackPtr()])) = 0;
}

// find the offset of the table record in the block
int TableBlock::FindRecordIndex(const char* table_name){
	if (this->RecordNum() == 0) return -1;
	int low = 0, mid, high = this->RecordNum()-1;
	while(low <= high){
		mid = (low + high) >> 1;
		int cmp = strcmp(table_name, (char*)&this->block_data[DATA_BEG + mid * TABLE_RECORD_SIZE]);
		if(cmp > 0) low = mid + 1;
		else if(cmp < 0) high = mid - 1;
		else return mid;
	}
	return low;
}

// given a table name, delete all the meta data of the table
// from the block, if table not found raise exception
void TableBlock::DropTable(const char* table_name){
	int table_index = this->FindRecordIndex(table_name);
	uint16_t table_addr = DATA_BEG + TABLE_RECORD_SIZE*table_index;
	if(table_index < 0 || strcmp(table_name, (char*)&this->block_data[table_addr])){
		throw TableNotFound(string(table_name));
	}
	uint16_t table_offset = DATA_BEG + table_index*TABLE_RECORD_SIZE;
	uint16_t attr_addr = *(uint16_t*)&this->block_data[table_offset + 32];
	uint8_t attr_num = *(uint8_t*)&this->block_data[table_offset + 34];
	uint16_t attr_size = attr_num * ATTR_RECORD_SIZE;
	uint16_t pointer = attr_addr;
	while(pointer - attr_size != this->StackPtr()){
		memcpy(&this->block_data[pointer - ATTR_RECORD_SIZE], &this->block_data[pointer - attr_size - ATTR_RECORD_SIZE], ATTR_RECORD_SIZE);
		pointer -= ATTR_RECORD_SIZE;
	}
	this->StackPtr() += attr_size;
	this->RecordNum()--;
	for(uint16_t i = table_index; i < this->RecordNum(); i++){
		memcpy((char*)&this->block_data[table_addr],(char*)&this->block_data[table_addr + TABLE_RECORD_SIZE], TABLE_RECORD_SIZE);
		table_addr += TABLE_RECORD_SIZE;
	}
	for(uint16_t i = 0; i < this->RecordNum(); i++){
		if(*(uint16_t*)&(this->block_data[DATA_BEG + i * TABLE_RECORD_SIZE + 32]) < attr_addr){
			*(uint16_t*)&(this->block_data[DATA_BEG + i * TABLE_RECORD_SIZE + 32]) += attr_size;
		}
	}
}

// get all the meta info of table
void TableBlock::GetTableMeta(const char* table_name, uint32_t & table_addr, uint32_t & table_index, uint8_t& attr_num, uint16_t & attr_addr, uint8_t& key_index){
	uint16_t index = this->FindRecordIndex(table_name);
	uint16_t offset = this->DATA_BEG + index * TABLE_RECORD_SIZE;
	if(index >= this->RecordNum() || strcmp(table_name, (char*)&this->block_data[offset])){
		throw TableNotFound(string(table_name));
	}
	attr_addr = *(uint16_t*)&this->block_data[offset + 32];
	attr_num = *(uint8_t*)&this->block_data[offset + 34];
	key_index = *(uint8_t*)&this->block_data[offset + 35];
	table_addr = *(uint32_t*)&this->block_data[offset + 36];
	table_index = *(uint16_t*)&this->block_data[offset + 40];
}

// get all the info of table attributes
void TableBlock::GetAttrMeta(char* attr_name, DBenum & attr_type, uint16_t attr_addr){
	attr_addr -= 32;
	strcpy(attr_name, (char*)&this->block_data[attr_addr]);
	attr_addr -= 2;
	attr_type = (DBenum)*(uint16_t*)&this->block_data[attr_addr];
}

// use a attributes type list to format block data
// make access element convient
void RecordBlock::Format(const DBenum* attr_type, uint16_t attr_num, unsigned short key){
	if(this->is_formated) return;
	this->tuple_size = 0;
	this->key_index = key;
	this->attr_num = attr_num;
	this->size = new unsigned short[attr_num];
	this->type = new DBenum[attr_num];
	for(unsigned short i = 0; i < attr_num; i++){
		this->size[i] = typeLen(attr_type[i]);
		this->type[i] = attr_type[i];
		this->tuple_size += size[i];
	}
	this->is_formated = true;
	// if(this->tuple_size > BLOCK_SIZE - DATA_BEG){
	// 	throw TooLargeTuple();
	// }
}

// given a data's row and colomn (row,colomn counts from 0)
// return its pointer
uint8_t* RecordBlock::GetDataPtr(unsigned short row, unsigned short colomn){
	unsigned short target_row_size = 0;
	for(unsigned short i = 0; i < colomn; i++){
		target_row_size += this->size[i];
	}
	return &(this->block_data[DATA_BEG + row*tuple_size + target_row_size]);
}

// binary search:
// if not found,
// return the smallest of the larger entry
int RecordBlock::FindTupleIndex(const void* key_data){
	if(this->RecordNum() == 0) return -1;
	int low = 0, mid, high = this->RecordNum()-1;
	while(low <= high){
		mid = (low + high) >> 1;
		int cmp = compare((uint8_t*)key_data,
				this->GetDataPtr(mid, this->key_index), this->type[this->key_index]);
		if(cmp > 0) low = mid + 1;
		else if(cmp < 0) high = mid - 1;
		else return mid;
	}
	return low;
}

// given a tuple's data list
// insert it into the right position in the block
int RecordBlock::InsertTuple(const void** data_list){
	int target_index = this->FindTupleIndex(data_list[this->key_index]);
	if (target_index < 0)  target_index = 0;
	return this->InsertTupleByIndex(data_list, target_index);
}

// insert a tuple into a specific position
int RecordBlock::InsertTupleByIndex(const void** data_list, int position){
	uint8_t* addr = this->GetDataPtr(this->RecordNum(), 0);
	for (unsigned short i = position; i < this->RecordNum(); i++) {
		memcpy(addr, addr - this->tuple_size, this->tuple_size);
		addr -= this->tuple_size;
	}
	addr = this->GetDataPtr(position, 0);
	for (unsigned short i = 0; i < this->attr_num; i++) {
		memcpy(addr, data_list[i], this->size[i]);
		addr += this->size[i];
	}
	this->RecordNum()++;
	return position;
}


// set the value of in a tuple(row, colomn)
void RecordBlock::SetTupleValue(unsigned short row, unsigned short colomn, const void* value){
	uint8_t* addr = this->GetDataPtr(row, colomn);
	memcpy(addr, value, this->size[colomn]);
}


// given a key and delete its corresponding tuple
// also fill the deleted tuple position with other tuple
// still keep the tuples ordered by key 
void RecordBlock::RemoveTuple(unsigned short row){
	uint8_t* addr = this->GetDataPtr(row, 0);
	for(unsigned short i = row; i < this->RecordNum(); i++){
		memcpy(addr, addr + this->tuple_size, this->tuple_size);
		addr += this->tuple_size;
	}
	this->RecordNum()--;
}