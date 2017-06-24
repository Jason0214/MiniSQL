
#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include <string>

// call Flush() after cout.
// Do not call cin, call GetString() / GetInt() / GetFloat() if necessary

void BeginQuery()
{

}

void ExeSelect(TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const ComparisonVector& cmpVec)
{
	//Assumption: the first operand is always attribute
	//the second is always a constant
	Catalog* catalog = &Catalog::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	int index = -1; //record the index
	string primaryKeyName = tableMeta->GetAttrName(tableMeta->primay_key_index);
	for (auto i = cmpVec.begin();i < cmpVec.end();i++) {
		if (primaryKeyName == (*i).Comparand1.Content) {
			index = tableMeta->primay_key_index;
			break;
		}
		else {
			//TODO
		}
	}
	
	//catalog->GetTableAttr()
	//catalog->GetIndex(TableAlias[sourceTableName],)
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

void ExeOutputTable(const TableAliasMap& tableAlias, const string& sourceTableName)
{
	// Print "end_result" in the last line to stop


	// if table on disk
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	unsigned short record_key = tableMeta->primay_key_index < 0 ? 0 : tableMeta->primay_key_index;	

	RecordBlock* result_block_ptr = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	result_block_ptr.Format(tableMeta->attr_type_list, tableMeta->attr_num, record_key);
	while(true){
		for(unsigned int i = 0; i < result_block_ptr->RecordNum(); i++){
			for(unsigned int j = 0; j < tableMeta->attr_num; j++){
				switch(tableMeta->attr_type_list[j]){
					case DB_TYPE_INT: `cout <<  *(int*)result_block_ptr->GetDataPtr(i, j); ` break;
					case DB_TYPE_FLOAT: `cout <<  *(float*)result_block_ptr->GetDataPtr(i, j); ` break;
					default: `cout <<  (char*)result_block_ptr->GetDataPtr(i, j); ` break;
				}
			}
		}
		uint32_t next = result_block_ptr->NextBlockIndex();
		if(next == 0){ 
			bufferManager.ReleaseBlock((Block* &)result_block_ptr);
			break;
		}
		bufferManager.ReleaseBlock((Block* &)result_block_ptr);
		result_block_ptr =  dynamic_cast<RecordBlock*>(bufferManager.GetBlock(next));
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