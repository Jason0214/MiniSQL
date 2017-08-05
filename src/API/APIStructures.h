
#ifndef _API_STRUCTURES_H_
#define _API_STRUCTURES_H_

#include <vector>
#include <map>
#include <string>

typedef std::string TableAlias;
typedef std::string TableName;
typedef std::map<TableAlias, TableName> TableAliasMap;

typedef struct  
{
	// TypeName can be "Attribute" | "int" | "float" | "string"
	std::string TypeName;
	std::string Content;
	std::string TableNameIfIsAttr;
} Comparand;

typedef struct  
{
	Comparand Comparand1;
	Comparand Comparand2;

	// Operation can be ">" | ">=" | "=" | "<" | "<=" | "<>"
	std::string Operation;
} Comparison;

typedef std::vector<Comparison> ComparisonVector;

typedef struct  
{
	std::string AttrName;
	std::string AttrAlias;
} AttrNameAlias;

typedef std::vector<AttrNameAlias> AttrNameAliasVector;

typedef std::vector<std::string> InsertValueVector;

typedef struct  
{
	std::string AttrName;
	std::string TypeName; // Can be char, varchar, int, float
	int TypeParam;
	bool bePrimaryKey;
	bool beUnique;
	bool beNotNull;
} AttrDefinition;

typedef std::vector<AttrDefinition> AttrDefinitionVector;

typedef struct
{
	uint32_t begin;
	uint32_t end;
} TraversalAddr;

#endif