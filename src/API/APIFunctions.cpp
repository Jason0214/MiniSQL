
#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include <string>


//may add to record manager
//check if a tuple is valid
inline bool checkTuple(RecordBlock* block, int line,TableMeta* tableMeta,const ComparisonVector& cmpVec) {
	std::string cmpOperator;
	for (auto i = cmpVec.begin();i < cmpVec.end(); i++) {
		std::string cmpOperator = (*i).Operation;
		//TODO
		for (unsigned int i = 0; i < tableMeta->attr_num; i++) {

			switch (tableMeta->attr_type_list[j]) {
			case DB_TYPE_INT: cout << *(int*)block->GetDataPtr(i, j);  break;
			case DB_TYPE_FLOAT: cout << *(float*)block->GetDataPtr(i, j);  break;
			default: cout << (char*)block->GetDataPtr(i, j);  break;
			}
		}	
	}	
}

// call Flush() after cout.
// Do not call cin, call GetString() / GetInt() / GetFloat() if necessary

void BeginQuery()
{

}

void ExeSelect(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const ComparisonVector& cmpVec)
{
	//Assumption: the first operand is always attribute
	//the second is always a constant

	//variables init
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	//make sure table name is valid
	std::string tableName;
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		throw(e);
	}
	TableMeta* tableMeta = catalog->GetTableMeta(tableName);

	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	newNameList = new std::string[tableMeta->attr_num];
	newTypeList = new DBenum[tableMeta->attr_num];
	for (int i = 0;i < tableMeta->attr_num;i++) {
		newNameList[i] = tableMeta->GetAttrName[i];
		newTypeList[i] = tableMeta->GetAttrType[i];
	}
	catalog->CreateTable(resultTableName, newNameList, newTypeList, tableMeta->attr_num, tableMeta->primay_key_index);
	RecordBlock* resultData = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));


	//try to find a proper index
	Block* indexRoot = nullptr;
	bool isPrimary = false; //if it is a primary index
	std::string primaryKeyName = tableMeta->GetAttrName(tableMeta->primay_key_index);
	for (auto i = cmpVec.begin();i < cmpVec.end();i++) {
		//check if the attribute is a primary index
		if (primaryKeyName == (*i).Comparand1.Content) {
			indexRoot = bufferManager->GetBlock(catalog->GetIndex(tableName,tableMeta->primay_key_index));
			isPrimary = true;
			break;
		}
		//check if the attribute is a secondary index
		else {
			for (int i = 0;i < tableMeta->attr_num;i++) {
				uint32_t index = catalog->GetIndex(tableName,i);
				if (index) {
					indexRoot = bufferManager->GetBlock(catalog->GetIndex(tableName, index));
				}
			}
		}
		//index found
		if (indexRoot) {
			
		}
		//no index found
		else {
			
		}
	}
}

void ExeProject(TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec)
{
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	Block* block = bufferManager->GetBlock(tableMeta->table_addr);
}

void ExeNaturalJoin(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{

}

void ExeCartesian(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{

}

void ExeOutputTable(TableAliasMap& tableAlias, const string& sourceTableName)
{
	// Print "end_result" in the last line to stop


	// if table on disk
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	unsigned short record_key = tableMeta->primay_key_index < 0 ? 0 : tableMeta->primay_key_index;	

	RecordBlock* result_block_ptr = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	result_block_ptr->Format(tableMeta->attr_type_list, tableMeta->attr_num, record_key);
	while(true){
		for(unsigned int i = 0; i < result_block_ptr->RecordNum(); i++){
			for(unsigned int j = 0; j < tableMeta->attr_num; j++){
				switch(tableMeta->attr_type_list[j]){
					case DB_TYPE_INT: cout <<  *(int*)result_block_ptr->GetDataPtr(i, j);  break;
					case DB_TYPE_FLOAT: cout <<  *(float*)result_block_ptr->GetDataPtr(i, j);  break;
					default: cout <<  (char*)result_block_ptr->GetDataPtr(i, j);  break;
				}
			}
		}
		uint32_t next = result_block_ptr->NextBlockIndex();
		if(next == 0){ 
			bufferManager->ReleaseBlock((Block* &)result_block_ptr);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)result_block_ptr);
		result_block_ptr =  dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}

	// if table is a temperary table not on disk
		//TODO
}

void EndQuery()
{

}

void ExeInsert(const std::string& tableName, InsertValueVector& values)
{
	// Print result information in one line
}

void ExeUpdate(const std::string& tableName, const std::string& attrName, const std::string& value)
{
	// Print result information in one line
}

void ExeDelete(const std::string& tableName, const ComparisonVector& cmpVec)
{
	// Print result information in one line
}

void ExeDropIndex(const std::string& tableName, const std::string& indexName)
{
	// Print result information in one line
}

void ExeDropTable(const std::string& tableName)
{
	// Print result information in one line
}

void ExeCreateIndex(const std::string& tableName, const std::string& attrName, const std::string& indexName)
{
	// Print result information in one line
}

void ExeCreateTable(const std::string& tableName, const AttrDefinitionVector& defVec)
{
	// Print result information in one line
}