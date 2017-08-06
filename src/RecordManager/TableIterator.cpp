#include "TableIterator.h"

#include <cassert>

#include "../BufferManager/BufferManager.h"

static BufferManager & buffer_manager =  BufferManager::Instance();

bool CheckEqual(const TemporalTable_Iterator & left_v, const TemporalTable_Iterator & right_v){
    return left_v.map_iter == right_v.map_iter;
}

bool CheckEqual(const MaterializedTable_Iterator & left_v, const MaterializedTable_Iterator & right_v){
    return (left_v.block_addr == right_v.block_addr) && (left_v.tuple_index == right_v.tuple_index);
}

MaterializedTable_Iterator::
~MaterializedTable_Iterator(){
    if(this->block_ptr){
        buffer_manager.ReleaseBlock(this->block_ptr);
        this->block_ptr = NULL;
    }
}

MaterializedTable_Iterator::
MaterializedTable_Iterator(uint32_t block_addr, 
                        int tuple_index,
                        int attr_num, 
                        DBenum * attr_type, 
                        int key_index)
                        :block_ptr(NULL),
                        block_addr(block_addr),
                        tuple_index(tuple_index),
                        key_index(key_index),
						attr_num(attr_num),
                        attr_type(attr_type){
    this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(block_addr));
    this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index); 
}

const MaterializedTable_Iterator & 
MaterializedTable_Iterator::operator=(const MaterializedTable_Iterator & right_v){
    if(this != &right_v){
        this->attr_num = right_v.attr_num;
        this->attr_type = right_v.attr_type;
        this->key_index = right_v.key_index;
        this->tuple_index = right_v.tuple_index;
		this->block_addr = right_v.block_addr;
        this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->block_addr));
        this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);         
    }
    return *this;
}  

void MaterializedTable_Iterator::next(){
    // if the index reach the end of current block, open the next one
	assert(this->block_addr != 0);
    if(this->tuple_index == this->block_ptr->RecordNum()){
        this->tuple_index = 0;
        this->block_addr = this->block_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock(this->block_ptr);
		if (this->block_addr == 0) {
			this->block_ptr = NULL;
		}
		else {
			this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->block_addr));
			this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
		}
    }
    else{
        this->tuple_index++;
    }
}