#ifndef __RECORD_MANAGER__
#define __RECORD_MANAGER__

#include <map>
#include <sstream>
#include "Table.h"

#define MAX_TEMPORAL_TABLE_SIZE (1 << 25) //32MB

class RecordManager{
// currently not support concurrency.
public:
	static RecordManager & Instance(){
		static RecordManager theRecordManager;
		return theRecordManager;
	}
	~RecordManager(){
		this->clear();
	}

	void clear(){
		std::map<std::string, Table*>::iterator iter;
		for(iter = this->data.begin(); iter != this->data.end(); iter++){
			delete iter->second;
		}
		this->data.clear();
	}
	void addTable(Table * table_ptr){
		this->data[table_ptr->getTableName()] = table_ptr;
	}
	Table* getTable(const std::string & table_name){
		return this->data.at(table_name);
	}

	static std::string Alias4IndirectAttr(const std::string & table_name, const std::string & attr_name, unsigned int seed){
		stringstream ss;
		ss << "_" << table_name << "_" << seed;
		return ss.str();
	}

	static void naturalJoin(DBenum join_type, Table* src_table1, Table* src_table2, Table* dst_table,
							const AttrAlias & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
		switch(join_type){
			case DB_DOUBLE_BLOCK_NESTED_JOIN: 
				doubleBlockNestedNaturalJoin(src_table1, src_table2, dst_table, attr_alias, commonAttrIndex); 
				break;
			case DB_SINGLE_BLOCK_NESTED_JOIN:
				break;
			case DB_NESTED_LOOP_JOIN:
				break;
		}
	}

	static void doubleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttrAlias & attr_alias,  const vector<pair<int,int> > & commonAttrIndex);

private:
	RecordManager(){};
	RecordManager(const RecordManager &);
	const RecordManager & operator=(const RecordManager &);

	std::map<std::string, Table*> data;
};


#endif