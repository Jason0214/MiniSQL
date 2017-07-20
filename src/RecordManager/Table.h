#ifndef __TABLE__
#define __TABLE__

#include <map>
#include "RecordStructures.h"

class Table{
public:
	Table(const std::string & table_name);

	Table(const std::string & table_name, 
		const Table & based_table, 
		const AttrAlias & attr_name_map, 
		TableImplement op);

	Table(const std::string & table_name, 
		const Table & based_table1, 
		const AttrAlias & attr_name_map1, 
		const Table & based_table2, 
		const AttrAlias & attr_name_map2,
		TableImplement op);

	void insertTule(const void** tuple_data){
		if(this->table_flag == DB_TEMPORAL){
			this->temporal_InsertTuple(tuple_data);
		}
		else{
			//DB_MATERIALIZED
			this->materialized_InsertTuple(tuple_data);
		}

		//TODO: check table size
	}

	void deleteTuple(){
		if(this->table_flag == DB_TEMPORAL){
			this->temporal_DeleteTuple(tuple_data);
		}
		else{
			//DB_MATERIALIZED
			this->materialized_DeleteTuple(tuple_data);
		}
	}

	void updateTuple(){
		if(this->table_flag == DB_TEMPORAL){
			this->temporal_UpdateTuple(tuple_data);
		}
		else{
			//DB_MATERIALIZED
			this->materialized_UpdateTuple(tuple_data);
		}
	}
	~Table(){
		delete [] table_meta;
		delete [] attr_type;
		if(this->cursor_block){
			BufferManager::Instance().ReleaseBlock(this->cursor_block);
		}
	}
	const std::string & getAttrName(int index) const{
		return this->attr_name[index];
	}
	const std::string & getKeyAttrName() const{
		return this->attr_name[this->key_index];
	}
	DBenum getAttrType(int index) const{
		return this->attr_type[index];
	}
private:
	void materialized_InsertTuple(const void** );
	void temporal_InsertTuple(const void** );

	DBenum table_flag;

	std::string table_name;
	DBenum* attr_type;
	std::string* attr_name;
	int attr_num;
	int key_index;
	bool is_primary_key;
	
	// list is only used for DB_TEMPORAL table
	std::map<TupleKey, Tuple> table_data;

	// only used for DB_MATERIALIZED table
	uint32_t index_addr;
	uint32_t data_addr;
};



#endif