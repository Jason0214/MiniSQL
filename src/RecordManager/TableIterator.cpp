#include "../BufferManager/BufferManager.h"
#include "TableIterator.h"
#include <cassert>

static BufferManager & buffer_manager =  BufferManager::Instanse();


bool operator==(const TemporalTable_Iterator & left_v, const TemporalTable_Iterator & right_v){
	return left_v.map_iter == right_v.iter;
}

bool operator==(const MaterializedTable_Iterator & left_v, const MaterializedTable_Iterator & right_v){
	return (left_v.block_addr == right_v.block_addr) && (left_v.tuple_index == right_v.tuple_index);
}

MaterializedTable_Iterator::~MaterializedTable_Iterator(){
	if(this->block_ptr){
		buffer_manager.ReleaseBlock(this->block_ptr);
		this->block_ptr = NULL;
	}
}

void TemporalTable_Iterator::next(){
	this->map_iter++;
}


void MaterializedTable_Iterator::next(){
	// get the block if never did before
	if(!this->block_ptr){
		this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->block_addr));
		this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
	}
	// if the index reach the end of current block, open the next one
	if(this->tuple_index == this->block_ptr->RecordNum()){
		this->tuple_index = 0;
		this->block_addr = this->block_ptr->NextBlockIndex();
		assert(this->block_addr != 0);
		buffer_manager.ReleaseBlock(this->block_ptr);
		this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->block_addr))
		this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
	}
	else{
		this->tuple_index++;
	}
}