
#ifndef _API_FUNCTIONS_H_
#define _API_FUNCTIONS_H_

#include "APIStructures.h"

void BeginQuery();

void ExeSelect(const TableAliasMap& tableAlias, const string& sourceTableName, 
	const string& resultTableName, const ComparisonVector& cmpVec);

void ExeProject(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec);

void ExeNaturalJoin(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName);

void ExeCartesian(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName);

void ExeOutputTable(const TableAliasMap& tableAlias, const string& sourceTableName);

void EndQuery();

void ExeInsert(const string& tableName, InsertValueVector& values);

void ExeUpdate(const string& tableName, const string& attrName, const string& value, const ComparisonVector& cmpVec);

void ExeDelete(const string& tableName, const ComparisonVector& cmpVec);

void ExeDropIndex(const string& tableName, const string& indexName);

void ExeDropTable(const string& tableName);

void ExeCreateIndex(const string& tableName, const string& attrName, const string& indexName);

void ExeCreateTable(const string& tableName, const AttrDefinitionVector& defVec);

#endif
