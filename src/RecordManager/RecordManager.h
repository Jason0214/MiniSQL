#ifndef __RECORD_MANAGER__
#define __RECORD_MANAGER__

#include "../IndexManager/IndexManager.h"
#include "../BufferManager/BufferManager.h"
#include "../CatalogManager/Catalog.h"
#include "../SharedFunc.h"

#include <list>
#include <cassert>

#define MAX_TEMPORAL_TABLE_SIZE (1 << 25) //32MB

class Tuple{
public:
	Tuple(){

	}
private:
	uint8_t* tuple_data;
};

class Table{
public:
	Table(const std::string & table_name)table_status(DB_MATERIALIZED){
		this->table_meta = BufferManager::Instance().GetTableMeta(table_name);	


	}
	Table(const std::string & table_name, TableMeta* table_meta):table_meta(NULL),table_status(DB_TEMPORAL){


	}
	void InsertTule(){

	}
	void DeleteTuple(){

	}
	Tuple & next(){

	}

	~Table(){
		delete table_meta;
		delete attr_type;
	}
	 GetTableName(){
		return table_meta->table_name;
	}
private:
	DBenum* attr_type;
	std::string table_name;
	int attr_num;
	DBenum table_status;

	// table_meta is set to NULL in temporal table
	TableMeta* table_meta;
	
	// list is only used for temporal tabl
	std::list<Tuple> table_data;
};


class RecordManager{
public:

private:

};


#endif