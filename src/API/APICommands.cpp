
#include "APICommands.h"
#include "APIFunctions.h"
#include "APIStructures.h"
#include "IO.h"
#include "../BufferManager/BufferManager.h"

using namespace std;

static string Command;

static void AcceptTableInfo(TableAliasMap& out_Result)
{
	int count;
	string alias;
	string tableName;

	count = GetInt();

	for (int i = 0; i < count; ++i)
	{
		alias = GetString();
		tableName = GetString();

		out_Result[alias] = tableName;
	}
}

static void AcceptComparand(Comparand& out_comparand)
{
	out_comparand.TypeName = GetString();
	out_comparand.Content = GetString();
	out_comparand.TableNameIfIsAttr = GetString();
}

static void AcceptComparison(Comparison& out_cmp)
{
	AcceptComparand(out_cmp.Comparand1);
	out_cmp.Operation = GetString();
	AcceptComparand(out_cmp.Comparand2);
}

static void AcceptComparisonVector(ComparisonVector& out_cmpVec)
{
	int count = GetInt();

	for (int i = 0; i < count; i++)
	{
		Comparison cmp;
		AcceptComparison(cmp);

		out_cmpVec.push_back(cmp);
	}
}

static void AcceptExeSelect(TableAliasMap& tableInfo)
{
	string sourceName;
	string resultName;
	ComparisonVector cmpVec; 

	sourceName = GetString();
	resultName = GetString();
	AcceptComparisonVector(cmpVec);

	ExeSelect(tableInfo, sourceName, resultName, cmpVec);
}

static void AcceptAttrNameAlias(AttrNameAlias& attr)
{
	attr.AttrName = GetString();
	attr.AttrAlias = GetString();
}

static void AcceptAttrNameAliasVector(AttrNameAliasVector& attrVec)
{
	int count = GetInt();

	for (int i = 0; i < count; i++)
	{
		AttrNameAlias attr;
		AcceptAttrNameAlias(attr);

		attrVec.push_back(attr);
	}
}

static void AcceptExeProject(TableAliasMap& tableInfo)
{
	string sourceName;
	string resultName;
	AttrNameAliasVector attrVec;

	sourceName = GetString();
	resultName = GetString();
	AcceptAttrNameAliasVector(attrVec);

	ExeProject(tableInfo, sourceName, resultName, attrVec);
}

static void AcceptExeNaturalJoin(TableAliasMap& tableInfo)
{
	string sourceName1, sourceName2, resultName;

	sourceName1 = GetString();
	sourceName2 = GetString();
	resultName = GetString();

	ExeNaturalJoin(tableInfo, sourceName1, sourceName2, resultName);
}

static void AcceptExeCartesian(TableAliasMap& tableInfo)
{
	string sourceName1, sourceName2, resultName;

	sourceName1 = GetString();
	sourceName2 = GetString();
	resultName = GetString();

	ExeCartesian(tableInfo, sourceName1, sourceName2, resultName);
}

static void AcceptExeOutputTable(TableAliasMap& tableInfo)
{
	string table = GetString();
	ExeOutputTable(tableInfo, table);
}

static void AcceptAttrDefinition(AttrDefinition& def)
{
	def.AttrName = GetString();
	def.TypeName = GetString();
	def.TypeParam = GetInt();
	def.bePrimaryKey = GetInt() == 1 ? true : false;
	def.beUnique = GetInt() == 1 ? true : false;
	def.beNotNull = GetInt() == 1 ? true : false;
}

static void AcceptAttrDefinitionVector(AttrDefinitionVector& defVec)
{
	int count = GetInt();

	for (int i = 0; i < count; i++)
	{
		AttrDefinition def;
		AcceptAttrDefinition(def);
		defVec.push_back(def);
	}
}

void AcceptQuery()
{
	TableAliasMap tableInfo;

	BeginQuery();
	
	while (1)
	{
		Command = GetString();

		if (Command == "end_query") return;
		else if (Command == "table_info") AcceptTableInfo(tableInfo);
		else if (Command == "select") AcceptExeSelect(tableInfo);
		else if (Command == "project") AcceptExeProject(tableInfo);
		else if (Command == "natural_join") AcceptExeNaturalJoin(tableInfo);
		else if (Command == "cartesian") AcceptExeCartesian(tableInfo);
		else if (Command == "get_result") AcceptExeOutputTable(tableInfo);
		else throw InvalidCommand(Command);
	}

	EndQuery();
}

void AcceptInsert()
{
	InsertValueVector values;

	string tableName = GetString();
	int count = GetInt();

	for (int i = 0; i < count; i++)
	{
		values.push_back(GetString());
	}

	ExeInsert(tableName, values);
}

void AcceptUpdate()
{
	string tableName = GetString();
	string attrName = GetString();
	string updateValue = GetString(); 
	ComparisonVector cmpVec;
	AcceptComparisonVector(cmpVec);
	ExeUpdate(tableName, attrName, updateValue, cmpVec);
}

void AcceptDelete()
{
	string tableName = GetString();
	ComparisonVector cmpVec;
	AcceptComparisonVector(cmpVec);
	ExeDelete(tableName, cmpVec);
}

void AcceptCreateTable()
{
	string tableName = GetString();
	AttrDefinitionVector def;
	AcceptAttrDefinitionVector(def);
	ExeCreateTable(tableName, def);
}

void AcceptDropTable()
{
	ExeDropTable(GetString());
}

void AcceptCreateIndex()
{
	ExeCreateIndex(GetString(), GetString(), GetString());
}

void AcceptDropIndex()
{
	ExeDropIndex(GetString(), GetString());
}

void OnQuit()
{
	BufferManager& buf = BufferManager::Instance();
	buf.WriteBackAll();
}