
#ifndef _API_FUNCTIONS_H_
#define _API_FUNCTIONS_H_

#include "APIStructures.h"

void BeginQuery();

void ExeSelect(const TableAliasMap& tableAlias, const std::string& sourceTableName, 
	const std::string& resultTableName, const ComparisonVector& cmpVec);

void ExeProject(const TableAliasMap& tableAlias, const std::string& sourceTableName,
	const std::string& resultTableName, const AttrNameAliasVector& attrVec);

void ExeNaturalJoin(const TableAliasMap& tableAlias, const std::string& sourceTableName1,
	const std::string& sourceTableName2, const std::string& resultTableName);

void ExeCartesian(const TableAliasMap& tableAlias, const std::string& sourceTableName1,
	const std::string& sourceTableName2, const std::string& resultTableName);

void ExeOutputTable(const TableAliasMap& tableAlias, const std::string& sourceTableName);

void EndQuery();

void ExeInsert(const std::string& tableName, InsertValueVector& values);

void ExeUpdate(const std::string& tableName, const std::string& attrName, const std::string& value, const ComparisonVector& cmpVec);

void ExeDelete(const std::string& tableName, const ComparisonVector& cmpVec);

void ExeDropIndex(const std::string& indexName);

void ExeDropTable(const std::string& tableName, bool echo = false);

void ExeCreateIndex(const std::string& tableName, const std::string& attrName, const std::string& indexName);

void ExeCreateTable(const std::string& tableName, const AttrDefinitionVector& defVec);

#endif
