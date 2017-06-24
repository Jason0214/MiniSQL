
#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include "../Type/ConstChar.h"
#include <string>
#include <sstream>

//may add to record manager
RecordBlock* insertTupleSafe(void** tuple, int attr_num, RecordBlock* srcBlock,
	int line, RecordBlock* dstBlock,BufferManager* bufferManager) {
	if (!dstBlock->CheckEmptySpace()) {
		RecordBlock* newBlock = dynamic_cast<RecordBlock*>(bufferManager->CreateBlock(DB_RECORD_BLOCK));
		dstBlock->NextBlockIndex() = newBlock->BlockIndex();
		bufferManager->ReleaseBlock((Block*&)(dstBlock));
		dstBlock = newBlock;
	}
	for (int i = 0;i < attr_num;i++) {
		tuple[i] = srcBlock->GetDataPtr(line,i);
		dstBlock->InsertTuple(tuple);
	}
	return dstBlock;
}
//may add to record manager
//compare template function
template <class T>
inline bool compare(const void* a, const void* b, const std::string &operation) {
	if (operation == ">") {
		return *(T*)a > *(T*)b;
	}
	else if (operation == ">=") {
		return *(T*)a >= *(T*)b;
	}
	else if (operation == "<") {
		return *(T*)a < *(T*)b;
	}
	else if (operation == "<=") {
		return *(T*)a <= *(T*)b;
	}
	else if (operation == "=") {
		return *(T*)a == *(T*)b;
	}
	else if (operation == "<>"||operation=="!=") {
		return *(T*)a != *(T*)b;
	}
	else {
		return false;
	}
}
//may add to record manager
//check if a tuple is valid
//cmpVec is sorted
inline bool checkTuple(RecordBlock* block, int line,TableMeta* tableMeta,const ComparisonVector& sortedCmpVec) {
	const ComparisonVector& cmpVec = sortedCmpVec;
	std::string cmpOperator;
	for (int i = 0,j = 0;cmpVec.begin()+i < cmpVec.end(); i++) {
		std::string cmpOperator = cmpVec[i].Operation;
		//TODO
		for (; j < tableMeta->attr_num; j++) {
			bool result=true;
			if (cmpVec[i].Comparand1.Content == tableMeta->GetAttrName(j)) {
				stringstream ss(cmpVec[i].Comparand2.Content);
				switch (tableMeta->attr_type_list[j]) {
					case DB_TYPE_INT:
						int num;
						ss >> num;
						result = compare<int>(block->GetDataPtr(line, j),&num,cmpOperator); 
						break;
					case DB_TYPE_FLOAT:
						float num;
						ss >> num;
						result = compare<float>(block->GetDataPtr(line, j), &num, cmpOperator);
						break;
					default:
						ConstChar<256> str;
						ss >> str;
						result = compare<ConstChar<256>>(block->GetDataPtr(line, j), &str, cmpOperator);
						break;
				}
				if (!result) return result;
			}
		}	
	}
	return true;
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
	void** tuple;
	tuple = new void*[tableMeta->attr_num];

	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	newNameList = new std::string[tableMeta->attr_num];
	newTypeList = new DBenum[tableMeta->attr_num];
	for (int i = 0;i < tableMeta->attr_num;i++) {
		newNameList[i] = tableMeta->GetAttrName(i);
		newTypeList[i] = tableMeta->GetAttrType(i);
	}
	catalog->CreateTable(resultTableName, newNameList, newTypeList, tableMeta->attr_num, tableMeta->primay_key_index);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, 0);

	//try to find a proper index
	Block* indexRoot = nullptr;
	bool isPrimary = false; //if it is a primary index
	std::string primaryKeyName = (tableMeta->primay_key_index >= 0) ?
		tableMeta->GetAttrName(tableMeta->primay_key_index) : "";
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
	}

	//index found
	if (indexRoot) {

	}
	//no index found
	else {
		RecordBlock* srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
		srcBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);
		while (true) {
			for (int i = 0; i < srcBlock->RecordNum(); i++) {
				//if the tuple fit the comparisonVector
				if (checkTuple(srcBlock, i, tableMeta, cmpVec)) {
					for (int j = 0;j < tableMeta->attr_num;j++) {
						tuple[j] = srcBlock->GetDataPtr(i, j);
					}
					//safe insert
					dstBlock = insertTupleSafe(tuple, tableMeta->attr_num, srcBlock, i, dstBlock, bufferManager);
				}
			}
			uint32_t next = srcBlock->NextBlockIndex();
			if (next == 0) {
				bufferManager->ReleaseBlock((Block* &)srcBlock);
				break;
			}
			bufferManager->ReleaseBlock((Block* &)srcBlock);
			srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
		}
	}
	//delete allocated memory
	bufferManager->ReleaseBlock((Block* &)dstBlock);
	delete[] tuple;
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