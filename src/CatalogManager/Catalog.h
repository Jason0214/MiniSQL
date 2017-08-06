#ifndef _Catalog_H_
#define _Catalog_H_
#include "../SharedFunc.h"
#include "../CONSTANT.h"
#include "../EXCEPTION.h"
#include "../IndexManager/IndexManager.h"
#include <string>

class TableMeta{
	// wrap all the needed meta information
	// about a TableMeta
public:
	TableMeta(const std::string & table_name):table_name(table_name),table_addr(0),attr_num(0),
			attr_name_list(NULL),attr_type_list(NULL),key_index(-1),primary_index_addr(0),is_primary_key(true){}
	~TableMeta(){
		delete [] this->attr_name_list;
		delete [] this->attr_type_list;
	}
	std::string table_name;
	// block address of the first block containing records
	uint32_t table_addr; 
	int attr_num; 
	// attribute name
	std::string* attr_name_list;
	// attribute type
	DBenum* attr_type_list;
	// key's index in the attributes. Records stores according to key
	int key_index; 
	// the root block address of the index of this table
	uint32_t primary_index_addr; 
	// if is_primary_key is set, then the key attribute is distinct
	bool is_primary_key; 
private:
	TableMeta(const TableMeta &);
	TableMeta & operator=(const TableMeta &);
};

class Catalog{
	// Singleton, has interfaces to get meta information
	// Database, Index, Table.
public:
	static Catalog & Instance(){
		static Catalog theCatalog;
		return theCatalog;
	}
	~Catalog(){}

	void CreateDatabase(const std::string & db_name);
	void UseDatabase(const std::string & db_name);

	void CreateTable(const std::string & table_name, std::string* attr_name_list, DBenum* attr_type_list, int attr_num, int & key_index);
	TableMeta* GetTableMeta(const std::string & table_name);
	void DropTable(const std::string & table_name);
	void DeleteTable(const std::string & table_name);

	void CreateIndex(const std::string & index_name, const std::string & table_name, const std::string & attr_name);
	uint32_t GetIndex(const std::string & table_name, int8_t secondary_key_index);
	void DropIndex(const std::string & index_name);

	// interface to change the root block of Index.
	// root of Index may change after insertion and deletion
	void UpdateTablePrimaryIndex(const std::string & table_name, uint32_t new_addr);
	void UpdateTableDataAddr(const std::string & table_name, uint32_t new_addr);
	void UpdateTableSecondaryIndex(const std::string & table_name, int8_t key_index, uint32_t new_addr);

	static RecordBlock* SplitRecordBlock(RecordBlock* origin_block_ptr, DBenum* types, int8_t num, int8_t key);
private:
	Catalog();
	Catalog(const Catalog&);
	Catalog & operator=(const Catalog&);

	TableBlock* SplitTableBlock(TableBlock* origin_block_ptr);

	void UpdateDatabaseInfo(const std::string & database_name, unsigned int info_type, uint32_t new_addr);
	void UpdateDatabaseTableIndex(const std::string & database_name, uint32_t new_addr){
		this->UpdateDatabaseInfo(database_name, 1, new_addr);
	}
	void UpdateDatabaseTableData(const std::string & database_name, uint32_t new_addr){
		this->UpdateDatabaseInfo(database_name, 0, new_addr);
	}
	void UpdateDatabaseIndexIndex(const std::string & database_name, uint32_t new_addr){
		this->UpdateDatabaseInfo(database_name, 3, new_addr);
	}
	void UpdateDatabaseIndexData(const std::string & database_name, uint32_t new_addr){
		this->UpdateDatabaseInfo(database_name, 2, new_addr);
	}
	
	RecordBlock* FindDatabaseBlock(const std::string & database_name);
	uint32_t FindTableBlock(const std::string & table_name);
	uint32_t FindIndexBlock(const std::string & table_name_mix_key);
	RecordBlock* FindIndexByName(const std::string & index_name);

	uint32_t database_block_addr;
	uint32_t user_block_addr;

	uint32_t table_data_addr;
	uint32_t table_index_addr;

	uint32_t index_data_addr;
	uint32_t index_index_addr;

	std::string current_database_name;
	bool database_selected;
};

#endif