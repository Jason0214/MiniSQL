#ifndef __TABLE_ITERATOR__
#define __TABLE_ITERATOR__

#include "../BufferManager/Block.h"
#include "RecordStructures.h"
class Table;

class TableIterator{
public:
    virtual ~TableIterator(){};
    virtual const void * getAttrData(int index) const = 0;
	virtual const void ** getDataList() const = 0;
    virtual void next() = 0;
    virtual bool isEqual(const TableIterator *) const = 0;
};

class TemporalTable_Iterator 
:public TableIterator{
public:
    TemporalTable_Iterator(TemporalTableDataMap::iterator & current_iter)
    :map_iter(current_iter){}
    TemporalTable_Iterator(const TemporalTable_Iterator & right_v){
        *this = right_v;
    }
    const TemporalTable_Iterator & operator=(const TemporalTable_Iterator & right_v){
        if(this != &right_v){
            this->map_iter = right_v.map_iter;
        }
        return *this;
    }
    virtual ~TemporalTable_Iterator(){}
    const void * getAttrData(int index) const{
        return map_iter->second[index];
    }
	const void ** getDataList() const {
		return map_iter->second.entry_ptr();
	}
    bool isEqual(const TableIterator * iter_ptr) const{
        return CheckEqual(*this, *dynamic_cast<const TemporalTable_Iterator*>(iter_ptr));
    }
    void next(){
        this->map_iter++;
    }
  
    friend bool CheckEqual(const TemporalTable_Iterator &, const TemporalTable_Iterator &);
  
	TemporalTableDataMap::iterator map_iter;
};

class MaterializedTable_Iterator
:public TableIterator{
public:
    MaterializedTable_Iterator(uint32_t block_addr, 
                            int tuple_index,
                            int attr_num, 
                            DBenum * attr_type, 
                            int key_index);
    MaterializedTable_Iterator(const MaterializedTable_Iterator & right_v){
        *this = right_v;
    }
    const MaterializedTable_Iterator & operator=(const MaterializedTable_Iterator & right_v); 
    virtual ~MaterializedTable_Iterator();
    const void * getAttrData(int index) const{
        return block_ptr->GetDataPtr(this->tuple_index, index);
    }
	const void ** getDataList() const {
		return (const void**)(block_ptr->GetDataPtr(this->tuple_index, 0));
	}
    bool isEqual(const TableIterator * iter_ptr) const{
        return CheckEqual(*this, *dynamic_cast<const MaterializedTable_Iterator*>(iter_ptr));
    }
    void next();
   
    friend bool CheckEqual(const MaterializedTable_Iterator &, const MaterializedTable_Iterator &);
    
    RecordBlock* block_ptr;
	uint32_t block_addr;
    int tuple_index;
    int key_index;
    int attr_num;
    DBenum* attr_type;
};


#endif