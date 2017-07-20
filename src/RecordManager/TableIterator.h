#ifndef __TABLE_ITERATOR__
#define __TABLE_ITERATOR__

#include "RecordStructures.h"
class Table;

class TableIterator{
public:
	virtual ~TableIterator(){};
	virtual void setCursor() = 0;
	virtual void next() = 0;
	virtual bool operator==(const TableIterator &) const = 0;
};

class TemporalTable_Iterator 
:public TableIterator{
public:
	TemporalTable_Iterator(std::map<TupleKey,Tuple>::iterator & current_iter)
	:map_iter(iterator){}
	
	virtual ~TemporalTable_Iterator(){}

	bool operator==(const TableIterator & iter) const{
		return *this == *dynamic_cast<TemporalTable_Iterator*>(&iter);
	}
	void next();
	friend bool operator==(const TemporalTable_Iterator &, const TemporalTable_Iterator &);
private:
	std::map<TupleKey,Tuple>::iterator map_iter;
};

class MaterializedTable_Iterator
:public TableIterator{
public:
	MaterializedTable_Iterator(uint32_t 
							block_addr, 
							int tuple_index,
							int attr_num, 
							DBenum * attr_type, 
							int key_index)
	:block_addr(block_addr),
	tuple_index(tuple_index),
	block_ptr(NULL),
	attr_type(attr_type),
	attr_num(attr_num),
	key_index(key_index){}
	
	virtual ~MaterializedTable_Iterator();
	
	bool operator==(const TableIterator & iter) const{
		return *this == *dynamic_cast<MaterializedTable_Iterator*>(&iter)
	}
	void next();
	friend bool operator==(const MaterializedTable_Iterator &, const MaterializedTable_Iterator &);
private:
	uint32_t block_addr;
	int tuple_index;
	int key_index;
	int attr_num;
	DBenum* attr_type;

	RecordBlock* block_ptr;
};


#endif