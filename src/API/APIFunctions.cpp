
#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"

// call Flush() after cout.
// Do not call cin, call GetString() / GetInt() / GetFloat() if necessary

void BeginQuery()
{

}

void ExeSelect(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const ComparisonVector& cmpVec)
{

}

void ExeProject(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec)
{

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

void ExeInsert(const string& tableName, InsertValueVector& values)
{
	// Print result information in one line
}

void ExeUpdate(const string& tableName, const string& attrName, const string& value)
{
	// Print result information in one line
}

void ExeDelete(const string& tableName, const ComparisonVector& cmpVec)
{
	// Print result information in one line
}

void ExeDropIndex(const string& tableName, const string& indexName)
{
	// Print result information in one line
}

void ExeDropTable(const string& tableName)
{
	// Print result information in one line
}

void ExeCreateIndex(const string& tableName, const string& attrName, const string& indexName)
{
	// Print result information in one line
}

void ExeCreateTable(const string& tableName, const AttrDefinitionVector& defVec)
{
	// Print result information in one line
}