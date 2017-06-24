
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