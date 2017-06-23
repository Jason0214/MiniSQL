#ifndef _Catalog_H_
#define _Catalog_H_
#include "../CONSTANT.h"
#include <string>

class TableMeta{
public:
	TableMeta(const std::string & table_name):table_name(table_name),attr_name_list(NULL),attr_type_list(NULL),
					attr_num(0),table_addr(0),primay_key_index(-1),primary_index_addr(0){}
	~TableMeta(){
		delete [] this->attr_name_list;
		delete [] this->attr_type_list;
	}
	const std::string table_name;
	uint32_t table_addr;
	int attr_num;
	std::string* attr_name_list;
	DBenum* attr_type_list;
	int primay_key_index;
	uint32_t primary_index_addr; // address of primary index
	std::string & GetAttrName(int index){return this->attr_name_list[index];}
	DBenum & GetAttrType(int index){return this->attr_type_list[index];}
};

typedef struct record_result_struct{
	RecordBlock* block_ptr;
	unsigned short index;
}RecordResult;

class Catalog{
public:
	static Catalog & Instance(){
		static Catalog theCatalog;
		return theCatalog;
	}
	~Catalog(){}

	void CreateDatabase(const std::string & db_name);
	void UseDatabase(const std::string & db_name);

	void CreateTable(const std::string & table_name, std::string* attr_name_list, DBenum* attr_type_list, int attr_num, int key_index);
	TableMeta* GetTableMeta(const std::string table_name);

	void CreateIndex(const string & index_name, const std::string & table_name, int8_t secondary_key_index);
	uint32_t Catalog::GetIndex(const std::string & table_name, int8_t secondary_key_index);

private:
	Catalog();
	Catalog(const Catalog&);
	Catalog & operator=(const Catalog&);

	TableBlock* SplitTableBlock(TableBlock* origin_block_ptr);
	RecordBlock* SplitRecordBlock(RecordBlock* origin_block_ptr, DBenum* types, int8_t num, int8_t key);

	void UpdateDatabaseTableIndex(const std::string & database_name, uint32_t new_addr);
	void UpdateDatabaseIndexIndex(const std::string & database_name, uint32_t new_addr);
	RecordResult* FindDatabaseBlock(const std::string & database_name);

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