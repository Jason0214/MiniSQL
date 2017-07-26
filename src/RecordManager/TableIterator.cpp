#include "../BufferManager/BufferManager.h"
#include "TableIterator.h"
#include <cassert>

static BufferManager & buffer_manager =  BufferManager::Instanse();


bool CheckEqual(const TemporalTable_Iterator & left_v, const TemporalTable_Iterator & right_v){
    return left_v.map_iter == right_v.iter;
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
                        :tuple_index(tuple_index),
                        block_ptr(NULL),
                        attr_type(attr_type),
                        attr_num(attr_num),
                        key_index(key_index){
    this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(block_addr));
    this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index); 
}

MaterializedTable_Iterator::
const MaterializedTable_Iterator & operator=(const MaterializedTable_Iterator & right_v){
    if(this != &right_v){
        this->attr_num = right_v.attr_num;
        this->attr_type = right_v.attr_type;
        this->key_index = right_v.key_index;
        this->tuple_index = right_v.tuple_index;
        this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(right_v.block_ptr->BlockIndex()));
        this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);         
    }
    return *this;
}  

void MaterializedTable_Iterator::next(){
    // if the index reach the end of current block, open the next one
    if(this->tuple_index == this->block_ptr->RecordNum()){
        this->tuple_index = 0;
        uint32_t next = this->block_ptr->NextBlockIndex();
        assert(next != 0);
        buffer_manager.ReleaseBlock(this->block_ptr);
        this->block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next))
        this->block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
    }
    else{
        this->tuple_index++;
    }
}