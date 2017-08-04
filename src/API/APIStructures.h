
#ifndef _API_STRUCTURES_H_
#define _API_STRUCTURES_H_

#include <vector>
#include <map>
#include <string>

typedef string TableAlias;
typedef string TableName;
typedef map<TableAlias, TableName> TableAliasMap;

typedef struct  
{
	// TypeName can be "Attribute" | "int" | "float" | "string"
	string TypeName;
	string Content;
	string TableNameIfIsAttr;
} Comparand;

typedef struct  
{
	Comparand Comparand1;
	Comparand Comparand2;

	// Operation can be ">" | ">=" | "=" | "<" | "<=" | "<>"
	string Operation;
} Comparison;

typedef vector<Comparison> ComparisonVector;

typedef struct  
{
	string AttrName;
	string AttrAlias;
} AttrNameAlias;

typedef vector<AttrNameAlias> AttrNameAliasVector;

typedef vector<string> InsertValueVector;

typedef struct  
{
	string AttrName;
	string TypeName; // Can be char, varchar, int, float
	int TypeParam;
	bool bePrimaryKey;
	bool beUnique;
	bool beNotNull;
} AttrDefinition;

typedef vector<AttrDefinition> AttrDefinitionVector;

typedef struct
{
	uint32_t begin;
	uint32_t end;
} TraversalAddr;

#endif