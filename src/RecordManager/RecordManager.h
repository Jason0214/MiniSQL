#ifndef __RECORD_MANAGER__
#define __RECORD_MANAGER__

#include <map>
#include <sstream>
#include "Table.h"
#include "../API/APIStructures.h"

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

	static std::string Alias4IndirectAttr(const std::string & table_name, 
										const std::string & attr_name, 
										unsigned int seed){
		stringstream ss;
		ss << "_" << table_name << "_" << seed;
		return ss.str();
	}

	static void CmpVec2TupleCmpVec(const Table* table, 
									const ComparisonVector& cmpVec, 
									TupleComparisonVector & dst_vector){
		TupleCmpInfo buf;
		for(unsigned int i = 0; i < cmpVec.size(); i++){
			for(int j = 0; j < table->getAttrNum(); j++){
				if(cmpVec[i].Comparand1.Content == table->getAttrName(j)){
					dst_vector.push_back(buf);
					dst_vector[dst_vector.size()-1].init(j, table->getAttrType(j), cmpVec[i].Comparand2.Content);
					break;
				}
			}
		}
	}

	static bool ConditionCheck(const void** tuple_data_list,
								const TupleComparisonVector & tuple_cmp_vec){
		for(unsigned int i = 0; i < tuple_cmp_vec.size(); i++){
			if(compare(tuple_cmp_vec[i].value, tuple_data_list[tuple_cmp_vec[i].origin_index], tuple_cmp_vec[i].type) != 0){
				return false;
			}	
		}
		return true;
	}

	static void naturalJoin(DBenum join_type, Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
		switch(join_type){
			case DB_DOUBLE_BLOCK_NESTED_JOIN: 
				doubleBlockNestedNaturalJoin(src_table1, src_table2, dst_table, attr_alias, commonAttrIndex); 
				break;
			case DB_SINGLE_BLOCK_NESTED_JOIN:
				singleBlockNestedNaturalJoin(src_table1, src_table2, dst_table, attr_alias, commonAttrIndex);
				break;
			case DB_NESTED_LOOP_JOIN:
				tupleNestedLoopNaturalJoin(src_table1, src_table2, dst_table, attr_alias, commonAttrIndex);
				break;
		}
	}

	static void join(DBenum join_type, Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias){
		switch(join_type){
			case DB_DOUBLE_BLOCK_NESTED_JOIN: 
				doubleBlockNestedJoin(src_table1, src_table2, dst_table, attr_alias); 
				break;
			case DB_SINGLE_BLOCK_NESTED_JOIN:
				singleBlockNestedJoin(src_table1, src_table2, dst_table, attr_alias);
				break;
			case DB_NESTED_LOOP_JOIN:
				tupleNestedLoopJoin(src_table1, src_table2, dst_table, attr_alias);
				break;
		}
	}

	static void thetaJoin(){
		//TODO
	}

private:
	RecordManager(){};
	RecordManager(const RecordManager &);
	const RecordManager & operator=(const RecordManager &);

	static void doubleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex);

	static void singleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex);

	static void tupleNestedLoopNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex);

	static void doubleBlockNestedJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias);

	static void singleBlockNestedJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias);

	static void tupleNestedLoopJoin(Table* src_table1, Table* src_table2, Table* dst_table,
							const AttributesAliasVector & attr_alias);

	std::map<std::string, Table*> data;
};


#endif