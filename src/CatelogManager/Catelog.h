#ifndef _CATELOG_H_
#define _CATELOG_H_
#include "../CONSTANT.h"
#include <string>

class Catelog{
public:
	static Catelog & Instance(){
		static Catelog theCatelog;
		return theCatelog;
	}
	~Catelog(){}

	void CreateDatabase(const std::string & db_name);	
	void UseDatabase(const std::string & db_name);
	void CreateTable(string & table_name, string* attr_name_list, DBenum* attr_type_list, unsigned int attr_num, int key_index);
	void GetTableAttr(string & table_name, uint32_t & table_addr, uint32_t & index_addr,
		string* attr_name_list, DBenum* attr_type_list, unsigned int & attr_num, unsigned int & key_index);
//TODO
	void CreateUser(const std::string user_name, const std::string passwd);

	bool AuthenUser();
	void SetPrivilege();

	int TupleCount();
	int TupleLen();
	int BlockCount();
	int BlockFactor(); // the number of distinct values that appear in the relation r for attribute A
	
private:
	Catelog();
	Catelog(const Catelog&);
	Catelog & operator=(const Catelog&);

	uint32_t database_block_addr;
	uint32_t index_block_addr;
	uint32_t user_block_addr;

	uint32_t table_data_addr;
	uint32_t table_index_addr;

	bool database_selected;
};

#endif