#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include "../Type/ConstChar.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

//#define __DEBUG__
#define __PRETTY__
#define INTLEN 10
#define FLOATLEN 12
#define STRLEN 15

//may add to record manager
RecordBlock* insertTupleSafe(const void** tuple, TableMeta* tableMeta, RecordBlock* dstBlock, BufferManager* bufferManager) {
	if (!dstBlock->CheckEmptySpace()) {
		RecordBlock* newBlock = dynamic_cast<RecordBlock*>(bufferManager->CreateBlock(DB_RECORD_BLOCK));
		dstBlock->NextBlockIndex() = newBlock->BlockIndex();
		bufferManager->ReleaseBlock((Block*&)(dstBlock));
		dstBlock = newBlock;
		dstBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->key_index);
	}
	dstBlock->InsertTuple(tuple);
	return dstBlock;
}
//may add to record manager
//compare template function
template <class T>
bool typedCompare(const void* a, const void* b, const std::string &operation) {
	//cout << *(T*)a << " " << *(T*)b << endl;
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
	else if (operation == "<>" || operation == "!=") {
		return *(T*)a != *(T*)b;
	}
	else {
		return false;
	}
}

bool compare(const void* a, const void* b, const std::string &operation, DBenum type) {
	bool result;
	switch (type) {
	case DB_TYPE_INT:
		result = typedCompare<int>(a, b, operation);
		break;
	case DB_TYPE_FLOAT:
		result = typedCompare<float>(a, b, operation);
		break;
	default:
		if (type - DB_TYPE_CHAR < 16) {
			result = typedCompare<ConstChar<16>>(a, b, operation);
		}
		else if (type - DB_TYPE_CHAR < 33) {
			result = typedCompare<ConstChar<33>>(a, b, operation);
		}
		else if (type - DB_TYPE_CHAR < 64) {
			result = typedCompare<ConstChar<64>>(a, b, operation);
		}
		else if (type - DB_TYPE_CHAR < 128) {
			result = typedCompare<ConstChar<128>>(a, b, operation);
		}
		else {
			result = typedCompare<ConstChar<256>>(a, b, operation);
		}
		break;
	}
	return result;
}

//may add to record manager
//check if a tuple is valid
//cmpVec is sorted
inline bool checkTuple(RecordBlock* block, int line, TableMeta* tableMeta, const ComparisonVector& sortedCmpVec) {
	const ComparisonVector& cmpVec = sortedCmpVec;
	for (int i = 0, j = 0;cmpVec.begin() + i < cmpVec.end(); i++) {
		std::string cmpOperator = cmpVec[i].Operation;
		for (; j < tableMeta->attr_num; j++) {
			if (cmpVec[i].Comparand1.Content == tableMeta->GetAttrName(j)) {
				bool result = true;
				stringstream ss(cmpVec[i].Comparand2.Content);
				DBenum type = tableMeta->attr_type_list[j];
				switch (type) {
				case DB_TYPE_INT:
					int integer;
					ss >> integer;
					result = typedCompare<int>(block->GetDataPtr(line, j), &integer, cmpOperator);
					break;
				case DB_TYPE_FLOAT:
					float num;
					ss >> num;
					result = typedCompare<float>(block->GetDataPtr(line, j), &num, cmpOperator);
					break;
				default:
					if (type - DB_TYPE_CHAR < 16) {
						ConstChar<16> str;
						ss >> str;
						result = compare(block->GetDataPtr(line, j), &str, cmpOperator, type);
					}
					else if (type - DB_TYPE_CHAR < 33) {
						ConstChar<33> str;
						ss >> str;
						result = compare(block->GetDataPtr(line, j), &str, cmpOperator, type);
					}
					else if (type - DB_TYPE_CHAR < 64) {
						ConstChar<64> str;
						ss >> str;
						result = compare(block->GetDataPtr(line, j), &str, cmpOperator, type);
					}
					else if (type - DB_TYPE_CHAR < 128) {
						ConstChar<128> str;
						ss >> str;
						result = compare(block->GetDataPtr(line, j), &str, cmpOperator, type);
					}
					else {
						ConstChar<256> str;
						ss >> str;
						result = compare(block->GetDataPtr(line, j), &str, cmpOperator, type);
					}
					break;
				}
				if (result) break;
				else return result;
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

//Assumption: the first operand is always attribute
//the second is always a constant
//cmpVec is sorted
void ExeSelect(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const ComparisonVector& cmpVec)
{
	int indexPos;
	std::string operation;
	//make sure table name is valid
	std::string tableName;
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName);
	}
	Table* src_table;
	try {
		src_table = record_manager.getTable(tableName);
	}
	catch (const exception &) {
		src_table = new table(tableName);
	}
	
	const void** tuple = new const void*[tableMeta->attr_num];
	RecordBlock* srcBlock;
	vector<Comparison> indexCmp;

	//create new temp table
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->key_index);

	//try to find a proper index
	Block* indexRoot = nullptr;
	DBenum indexType;
	std::string indexContent;
	bool isPrimary = false; //if it is a primary index
	std::string primaryKeyName = (tableMeta->key_index >= 0) ?
		tableMeta->GetAttrName(tableMeta->key_index) : "";
	for (auto i = cmpVec.begin();i < cmpVec.end();i++) {
		//check if the attribute is a primary index
		if (primaryKeyName == (*i).Comparand1.Content) {
			indexRoot = bufferManager->GetBlock(tableMeta->primary_index_addr);
			indexType = tableMeta->GetAttrType(tableMeta->key_index);
			indexContent = (*i).Comparand1.Content;
			indexPos = i - cmpVec.begin();
			isPrimary = true;
			operation = (*i).Operation;
			break;
		}
		//check if the attribute is a secondary index
		/*else {
		for (int j = 0;j < tableMeta->attr_num;j++) {
		uint32_t index = catalog->GetIndex(tableName,j);
		if (index) {
		indexRoot = bufferManager->GetBlock(catalog->GetIndex(tableName, index));
		indexType = tableMeta->GetAttrType(j);
		operation = (*i).Operation;
		indexContent = (*i).Comparand1.Content;
		indexPos = i - cmpVec.begin();
		}
		}
		}*/
	}

	//primary index found
	if (isPrimary) {
		indexCmp.push_back(cmpVec[indexPos]);
		indexCmp[0].Operation = ">";
	}
	if (isPrimary && (operation == ">" || operation == "=" || operation == ">=")) {
		uint32_t ptr; // the result block's index
		stringstream ss(indexContent);
		SearchResult* pos;
		indexManager = getIndexManager(indexType);
		switch (indexType) {
		case DB_TYPE_INT:
			int integer;
			ss >> integer;
			pos = indexManager->searchEntry(indexRoot, BPTree, &integer);
			break;
		case DB_TYPE_FLOAT:
			float num;
			ss >> num;
			pos = indexManager->searchEntry(indexRoot, BPTree, &num);
			break;
		default:
			pos = indexManager->searchEntry(indexRoot, BPTree, (void*)indexContent.c_str());
			break;
		}
		//if the b+tree is empty, pos will be nullptr
		if (pos) {
			ptr = *(pos->ptrs + pos->index);
			srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(ptr));
			delete pos;
		}
	}
	//no primary index found
	else {
		srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	}
	while (true) {
		srcBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->key_index);
		for (int i = 0; i < srcBlock->RecordNum(); i++) {
			//if the current key is already larger than the searched primary index
			if (isPrimary && (operation == "<" || operation == "=" || operation == "<=")) {
				if (checkTuple(srcBlock, i, tableMeta, indexCmp)) {
					break;
				}
			}
			if (*(int*)srcBlock->GetDataPtr(i, 0) == 254) {
				//	cout << "catch" << endl;
			}
			//if the tuple fit the comparisonVector
			if (checkTuple(srcBlock, i, tableMeta, cmpVec)) {
				for (int j = 0;j < tableMeta->attr_num;j++) {
					tuple[j] = srcBlock->GetDataPtr(i, j);
				}
				//safe insert
				dstBlock = insertTupleSafe(tuple, tableMeta, dstBlock, bufferManager);
			}
		}
		uint32_t next = srcBlock->NextBlockIndex();
		if (next == 0) {
			break;
		}
		bufferManager->ReleaseBlock((Block* &)srcBlock);
		srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}
	//delete allocated memory
	bufferManager->ReleaseBlock((Block* &)srcBlock);
	bufferManager->ReleaseBlock((Block* &)dstBlock);
	if (indexRoot) bufferManager->ReleaseBlock((Block* &)indexRoot);
	if (indexManager) delete indexManager;
	delete tableMeta;
	delete[] tuple;
	//drop the srcTable if it's a temp table
	std::regex e("^_tmp.*");
	if (std::regex_match(tableName, e)) {
		ExeDropTable(tableName);
	}
#ifdef __DEBUG__
	cout << "select result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}


//attrVec is sorted
void ExeProject(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec)
{
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	std::string tableName;
	//check if alias exists
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName);
	}
	TableMeta* tableMeta;
	try {
		tableMeta = catalog->GetTableMeta(tableName);
	}
	catch (const TableNotFound &) {
		cout << "Table `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	Block* block = bufferManager->GetBlock(tableMeta->table_addr);
	std::vector<int> attrIndexVec;
	//get attr index
	for (int i = 0, j = 0;i < tableMeta->attr_num&&j < (int)attrVec.size();i++) {
		if (tableMeta->GetAttrName(i) == attrVec[j].AttrName) {
			attrIndexVec.push_back(i);
			j++;
		}
	}
	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	newNameList = new std::string[attrIndexVec.size()];
	newTypeList = new DBenum[attrIndexVec.size()];
	for (int i = 0;i < (int)attrIndexVec.size();i++) {
		newNameList[i] = tableMeta->GetAttrName(attrIndexVec[i]);
		newTypeList[i] = tableMeta->GetAttrType(attrIndexVec[i]);
	}
	int keyIndex = 0;
	catalog->CreateTable(resultTableName, newNameList, newTypeList, attrIndexVec.size(), keyIndex);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(newTypeList, attrIndexVec.size(), keyIndex);
	TableMeta* newTableMeta = catalog->GetTableMeta(resultTableName);

	//insert data into new table
	RecordBlock* srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	const void** tuple = (const void**)(new void*[attrIndexVec.size()]);
	while (true) {
		srcBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->key_index);
		for (int i = 0; i < srcBlock->RecordNum(); i++) {
			for (int j = 0;j < (int)attrIndexVec.size();j++) {
				tuple[j] = srcBlock->GetDataPtr(i, attrIndexVec[j]);
			}
			//safe insert
			dstBlock = insertTupleSafe(tuple, newTableMeta, dstBlock, bufferManager);
		}
		uint32_t next = srcBlock->NextBlockIndex();
		if (next == 0) {
			break;
		}
		bufferManager->ReleaseBlock((Block* &)srcBlock);
		srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}
	//delete allocated memory
	bufferManager->ReleaseBlock((Block* &)srcBlock);
	bufferManager->ReleaseBlock((Block* &)dstBlock);
	delete tableMeta;
	delete newTableMeta;
	delete[] tuple;
	delete[] newNameList;
	delete[] newTypeList;
	//drop the srcTable if it's a temp table
	std::regex e("^_tmp.*");
	if (std::regex_match(tableName, e)) {
		ExeDropTable(tableName);
	}
#ifdef __DEBUG__
	cout << "project result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

//attr is sorted
void ExeNaturalJoin(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{
	//variables init
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	std::string cmpOperator = "=";
	//make sure table name is valid
	std::string tableName1, tableName2;
	try {
		tableName1 = tableAlias.at(sourceTableName1);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName1);
	}
	try {
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName2);
	}
	TableMeta* tableMeta1 = catalog->GetTableMeta(tableName1), *tableMeta2 = catalog->GetTableMeta(tableName2);
	//const void** tuple = (const void**)(new void*[tableMeta->attr_num]);
	RecordBlock* srcBlock1, *srcBlock2;
	std::vector<Comparison> indexCmp;
	std::vector<int> commonAttrIndex1, commonAttrIndex2;

	//get common attrs
	//attr_num is sorted
	bool isPrimary = false; //if a common attr in the primary index of both tables
	for (int i = 0, j = 0;i < tableMeta1->attr_num&&j < tableMeta2->attr_num;) {
		if (tableMeta1->GetAttrName(i) > tableMeta2->GetAttrName(j)) j++;
		else if (tableMeta1->GetAttrName(i) < tableMeta2->GetAttrName(j)) i++;
		else {
			if (tableMeta1->key_index == i&&tableMeta2->key_index == j) {
				isPrimary = true; //use primary index to reduce time complexity to O(n)
			}
			commonAttrIndex1.push_back(i);
			commonAttrIndex2.push_back(j);
			i++;j++;
		}
	}

	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	int newListSize = tableMeta1->attr_num + tableMeta2->attr_num - commonAttrIndex1.size();
	const void** tuple = (const void**)(new void*[newListSize]);
	newNameList = new std::string[newListSize];
	newTypeList = new DBenum[newListSize];
	//create new info lists
	//attr_num is sorted
	for (int i = 0, j = 0, k = 0;i < tableMeta1->attr_num || j < tableMeta2->attr_num;k++) {
		if (i >= tableMeta1->attr_num || tableMeta1->GetAttrName(i) > tableMeta2->GetAttrName(j)) {
			newNameList[k] = tableMeta2->GetAttrName(j);
			newTypeList[k] = tableMeta2->GetAttrType(j);
			j++;
		}
		else if (j >= tableMeta2->attr_num || tableMeta1->GetAttrName(i) < tableMeta2->GetAttrName(j)) {
			newNameList[k] = tableMeta1->GetAttrName(i);
			newTypeList[k] = tableMeta1->GetAttrType(i);
			i++;
		}
		else {
			newNameList[k] = tableMeta1->GetAttrName(i);
			newTypeList[k] = tableMeta1->GetAttrType(i);
			i++;j++;
		}
	}
	int keyIndex = 0;
	catalog->CreateTable(resultTableName, newNameList, newTypeList, newListSize, keyIndex);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(newTypeList, newListSize, keyIndex);
	TableMeta* newTableMeta = catalog->GetTableMeta(resultTableName);

	//insert data into new table
	srcBlock1 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta1->table_addr));
	srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta2->table_addr));
	//first loop:srcBlock1
	while (true) {
		srcBlock1->Format(tableMeta1->attr_type_list, tableMeta1->attr_num, tableMeta1->key_index);
		for (int i = 0; i < srcBlock1->RecordNum(); i++) {
			//reset srcBlock2
			srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta2->table_addr));
			//second loop:srcBlock2
			while (true) {
				srcBlock2->Format(tableMeta2->attr_type_list, tableMeta2->attr_num, tableMeta2->key_index);
				for (int j = 0; j < srcBlock2->RecordNum(); j++) {
					bool result = true; //compare result
					for (int k = 0;k < (int)commonAttrIndex1.size();k++) {
						DBenum type = tableMeta1->attr_type_list[commonAttrIndex1[k]];
						result = compare(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
							srcBlock2->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator, type);
						if (!result) break;
					}
					//join two tuples
					if (result) {
						for (int ii = 0, jj = 0, kk = 0;ii < tableMeta1->attr_num || jj < tableMeta2->attr_num;kk++) {
							if (ii >= tableMeta1->attr_num || tableMeta1->GetAttrName(ii) > tableMeta2->GetAttrName(jj)) {
								tuple[kk] = srcBlock2->GetDataPtr(j, jj);
								jj++;
							}
							else if (jj >= tableMeta2->attr_num || tableMeta1->GetAttrName(ii) < tableMeta2->GetAttrName(jj)) {
								tuple[kk] = srcBlock1->GetDataPtr(i, ii);
								ii++;
							}
							else {
								tuple[kk] = srcBlock1->GetDataPtr(i, ii);
								ii++;jj++;
							}
						}
						//insert the joined data
						dstBlock = insertTupleSafe(tuple, newTableMeta, dstBlock, bufferManager);
					}
				}
				uint32_t next = srcBlock2->NextBlockIndex();
				if (next == 0) {
					bufferManager->ReleaseBlock((Block* &)srcBlock2);
					break;
				}
				bufferManager->ReleaseBlock((Block* &)srcBlock2);
				srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
			}
		}
		uint32_t next = srcBlock1->NextBlockIndex();
		if (next == 0) {
			bufferManager->ReleaseBlock((Block* &)srcBlock1);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)srcBlock1);
		srcBlock1 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}

	//delete allocated memory
	bufferManager->ReleaseBlock((Block* &)dstBlock);
	delete tableMeta1;
	delete tableMeta2;
	delete newTableMeta;
	delete[] tuple;
	delete[] newNameList;
	delete[] newTypeList;
	//drop the srcTable if it's a temp table
	std::regex e("^_tmp.*");
	if (std::regex_match(tableName1, e)) {
		ExeDropTable(tableName1);
	}
	if (std::regex_match(tableName2, e)) {
		ExeDropTable(tableName2);
	}
#ifdef __DEBUG__
	cout << "Natural join result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

void ExeCartesian(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{
	//variables init
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	//make sure table name is valid
	std::string tableName1, tableName2;
	try {
		tableName1 = tableAlias.at(sourceTableName1);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName1);
	}
	try {
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName2);
	}
	TableMeta* tableMeta1 = catalog->GetTableMeta(tableName1), *tableMeta2 = catalog->GetTableMeta(tableName2);
	RecordBlock* srcBlock1, *srcBlock2;

	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	int newListSize = tableMeta1->attr_num + tableMeta2->attr_num;
	const void** tuple = (const void**)(new void*[newListSize]);
	newNameList = new std::string[newListSize];
	newTypeList = new DBenum[newListSize];
	//create new info lists
	//sourceTableName is sorted
	for (int i = 0;i < tableMeta1->attr_num;i++) {
		newNameList[i] = sourceTableName1 + "." + tableMeta1->GetAttrName(i);
		newTypeList[i] = tableMeta2->GetAttrType(i);
	}
	for (int i = tableMeta1->attr_num;i < newListSize;i++) {
		newNameList[i] = sourceTableName2 + "." + tableMeta2->GetAttrName(i - tableMeta1->attr_num);
		newTypeList[i] = tableMeta2->GetAttrType(i - tableMeta1->attr_num);
	}

	int keyIndex = 0;
	catalog->CreateTable(resultTableName, newNameList, newTypeList, newListSize, keyIndex);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(newTypeList, newListSize, keyIndex);
	TableMeta* newTableMeta = catalog->GetTableMeta(resultTableName);

	//insert data into new table
	srcBlock1 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta1->table_addr));
	srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta2->table_addr));
	//first loop:srcBlock1
	while (true) {
		srcBlock1->Format(tableMeta1->attr_type_list, tableMeta1->attr_num, tableMeta1->key_index);
		for (int i = 0; i < srcBlock1->RecordNum(); i++) {
			//reset srcBlock2
			srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta2->table_addr));
			//second loop:srcBlock2
			while (true) {
				srcBlock2->Format(tableMeta2->attr_type_list, tableMeta2->attr_num, tableMeta2->key_index);
				for (int j = 0; j < srcBlock2->RecordNum(); j++) {

					//join two tuples
					for (int ii = 0;ii < tableMeta1->attr_num;ii++) {
						tuple[ii] = srcBlock1->GetDataPtr(i, ii);
					}
					for (int ii = tableMeta1->attr_num;ii < newListSize;ii++) {
						tuple[ii] = srcBlock2->GetDataPtr(j, ii - tableMeta1->attr_num);
					}
					//insert the product data
					dstBlock = insertTupleSafe(tuple, newTableMeta, dstBlock, bufferManager);
				}
				uint32_t next = srcBlock2->NextBlockIndex();
				if (next == 0) {
					bufferManager->ReleaseBlock((Block* &)srcBlock2);
					break;
				}
				bufferManager->ReleaseBlock((Block* &)srcBlock2);
				srcBlock2 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
			}
		}
		uint32_t next = srcBlock1->NextBlockIndex();
		if (next == 0) {
			bufferManager->ReleaseBlock((Block* &)srcBlock1);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)srcBlock1);
		srcBlock1 = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}
	//delete allocated memory
	bufferManager->ReleaseBlock((Block* &)dstBlock);
	delete tableMeta1;
	delete tableMeta2;
	delete newTableMeta;
	delete[] tuple;
	delete[] newNameList;
	delete[] newTypeList;
	//drop the srcTable if it's a temp table
	std::regex e("^_tmp.*");
	if (std::regex_match(tableName1, e)) {
		ExeDropTable(tableName1);
	}
	if (std::regex_match(tableName2, e)) {
		ExeDropTable(tableName2);
	}
#ifdef __DEBUG__
	cout << "Cartesian product result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

void EndQuery()
{

}

//print out a table
void ExeOutputTable(const TableAliasMap& tableAlias, const string& sourceTableName)
{
	Table* table = record_manager.getTable(sourceTableName);
	
	//adjust align
	std::cout << std::left;
	//print out attr name
	int borderLen = tableMeta->attr_num - 1;
	for (int i = 0;i < tableMeta->attr_num;i++) {
		switch (tableMeta->attr_type_list[i]) {
		case DB_TYPE_INT: borderLen += INTLEN;  break;
		case DB_TYPE_FLOAT: borderLen += FLOATLEN;  break;
		default:  borderLen += STRLEN;  break;
		}
	}
	std::string horizontalBorder(borderLen, '-');
	horizontalBorder = " " + horizontalBorder + " ";
	std::cout << horizontalBorder << std::endl;
	for (int i = 0; i < table->getAttrNum(); i++) {
		std::cout << "|";
		switch (table->getAttrType(i)) {
		case DB_TYPE_INT: std::cout << std::setw(INTLEN);  break;
		case DB_TYPE_FLOAT: std::cout << std::setw(FLOATLEN);  break;
		default: std::cout << std::setw(STRLEN);  break;
		}
		std::cout << table->getAttrName(i);
	}
	std::cout << "|" << std::endl << horizontalBorder << std::endl;
	
	//print out data
	AutoPtr<TableIterator> iter(table->begin());
	AutoPtr<TableIterator> end(table->end());
	for(;iter->isEqual(end.raw_ptr); iter->next()){
		for (int j = 0; j < table->getAttrNum(); j++) {
			std::cout << "|";
			switch (table->getAttrType(j)) {
			case DB_TYPE_INT: std::cout << std::setw(INTLEN) << *(int*)iter->GetAttrData(j);  break;
			case DB_TYPE_FLOAT: std::cout << std::setw(FLOATLEN) << *(float*)iter->GetAttrData(j);  break;
			default: std::cout << std::setw(STRLEN) << (char*)iter->GetAttrData(j);  break;
			}
		}
		std::cout << "|";
		cout << endl;
	}
	std::cout << horizontalBorder << std::endl;
	std::cout << "end_result" << std::endl;
	Flush();
}

void ExeInsert(const std::string& tableName, InsertValueVector& values) {
	AutoPtr<Table> table(new Table(tableName));

	if (table->getAttrNum() != values.size()) {
		throw AttrNumberUnmatch("");
	}

	uint8_t ** tuple = new uint8_t*[table->getAttrNum()];
	for(int i = 0; i < table->getAttrNum(); i++){
		DBenum type = table->getAttrType(i);
		tuple[i] = new uint8_t[typeLen(type)];
		string2Bytes(value, type, tuple[i]);
	}
	table->insertTuple(tuple);
	for(int i = 0; i < table->getAttrNum(); i++){
		delete [] tuple[i];
	}
	delete [] tuple;
	cout << "1 Row Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

static void DeleteTableBlock(Table* table_ptr, RecordBlock* data_block_ptr){
	// if only one record remained in the block
	if (data_block_ptr->PreBlockIndex() == 0 && data_block_ptr->NextBlockIndex() == 0) {}
	else if (data_block_ptr->PreBlockIndex() == 0) {
		catalog_manager.UpdateTableDataAddr(tableName, data_block_ptr->NextBlockIndex());
		table_ptr->updateDataBlockAddr(data_block_ptr->NextBlockIndex());
		Block* next_block_ptr = buffer_manager->GetBlock(data_block_ptr->NextBlockIndex());
		next_block_ptr->PreBlockIndex() = 0;
		next_block_ptr->is_dirty = true;
		buffer_manager->ReleaseBlock(next_block_ptr);
		buffer_manager->DeleteBlock(data_block_ptr);
	}
	else if (data_block_ptr->NextBlockIndex() == 0) {
		Block* pre_block_ptr = buffer_manager->GetBlock(data_block_ptr->PreBlockIndex());
		pre_block_ptr->NextBlockIndex() = 0;
		pre_block_ptr->is_dirty = true;
		buffer_manager->ReleaseBlock(pre_block_ptr);
		buffer_manager->DeleteBlock(data_block_ptr);
	}
	else {
		Block* pre_block_ptr = buffer_manager->GetBlock(data_block_ptr->PreBlockIndex());
		Block* next_block_ptr = buffer_manager->GetBlock(data_block_ptr->NextBlockIndex());
		pre_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
		next_block_ptr->PreBlockIndex() = pre_block_ptr->BlockIndex();
		pre_block_ptr->is_dirty = true;
		next_block_ptr->is_dirty = true;
		buffer_manager->ReleaseBlock(next_block_ptr);
		buffer_manager->ReleaseBlock(pre_block_ptr);
		buffer_manager->DeleteBlock(data_block_ptr);
	}
}

void ExeUpdate(const std::string& tableName, const std::string& attrName,
	const std::string& value, const ComparisonVector& cmpVec)
{
	AutoPtr<Table> table(new Table(tableName));
	int updated_tuple_count = 0;

	// find attribute index and make sure it exists in the tab;e
	int attr_index = -1;
	for (int i = 0; i < table->getAttrNum(); i++) {
		if (table->getAttrName(i) == attrName) {
			attr_index = i;
			break;
		}
	}
	if (attr_index == -1) {
		throw AttributeNotFound(attrName);
	}

	DBenum key_type = table->getAttrType(table->getKeyIndex());
	uint8_t* raw_value = new uint8_t[table->getAttrNum()];
	string2Bytes(value, key_type, raw_value);

	// test if primary index exists in the cmpVec
	int key_match = -1;
	for (unsigned int i = 0; i < cmpVec.size(); i++) {
		if (cmpVec[i].Comparand1.TypeName == "Attribute" &&
			table->getAttrName(table->getKeyIndex()) == cmpVec[i].Comparand1.Content) {
			key_match = i;
		}
	}

	// begin and end address for deletion traversal
	uint32_t begin_addr = table->getDataBlockAddr(), end_addr = 0;
	if (key_match != -1) {
		table->BlockFilter(cmpVec[key_match].Operation, raw_value, &begin_addr, &end_addr);
	}

	DBenum attr_type = table->getAttrType(attr_index);
	uint32_t index_root = table->getIndexRoot();
	if (attr_index != table->getKeyIndex()) {
		// no need to update primary index
		uint32_t next_block_addr = begin_addr;
		while (next_block_addr != end_addr) {
			RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next_block_addr));
			data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
			int record_num = data_block_ptr->RecordNum();
			for (int i = 0; i < record_num; i++) {
				if (checkTuple(data_block_ptr, i, table->getAttrTypeList(), cmpVec)) {
					data_block_ptr->SetTupleValue(i, attr_index, raw_value);
					updated_tuple_count++;
				}
			}
			next_block_addr = data_block_ptr->NextBlockIndex();
			buffer_manager->ReleaseBlock(data_block_ptr);
		}
	}
	else {
		//pre-check whether the new attribute value would cause duplicated primary key
		if (table->isPrimaryKey()) {
			uint32_t target_block_addr;
			AutoPtr<SearchResult> result_ptr(index_manager_ptr->searchEntry(DB_BPTREE_INDEX, index_root, key_type, (void*)temp_ptr));
			if (compare(result_ptr->node->getKey(result_ptr->index), raw_value, key_type) == 0) {
				target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
			}
			else {
				if (block_entry->index != 0) {
					result_ptr->index--;
				}
				target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
			}
			BlockPtr<RecordBlock> target_block_ptr(dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_addr)));
			target_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			int target_index = target_block_ptr->FindTupleIndex(raw_value);
			if (target_index >= 0 && target_index < target_block_ptr->RecordNum() &&
				compare(raw_value, target_block_ptr->GetDataPtr(target_index, table_meta->key_index), key_type) == 0) {
				throw DuplicatedPrimaryKey("");
			}
		}

		/* update begins here */
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(begin_addr));
		uint8_t* buf = new uint8_t[data_block_ptr->tuple_size];
		const void ** tuple = new const void*[table->getAttrNum()];
		for (int i = 0; i < table->getAttrNum(); i++) {
			tuple[i] = buf + (data_block_ptr->GetDataPtr(0, i) - data_block_ptr->GetDataPtr(0, 0));
		}
		data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
		while (true) {
			RecordBlock* next_block_ptr = NULL;
			if (data_block_ptr->NextBlockIndex() != 0) {
				next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(data_block_ptr->NextBlockIndex()));
				next_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
			}
			int record_num = data_block_ptr->RecordNum();

			// tempararily remove the index
			SearchResult* data_block_entry = index_manager_ptr->searchEntry(DB_BPTREE_INDEX, index_root, key_type, data_block_ptr->GetDataPtr(0, attr_index));
			index_root = index_manager_ptr->removeEntry(DB_BPTREE_INDEX, index_root, key_type, data_block_entry);
			delete data_block_entry;

			for (int i = record_num - 1; i >= 0; i--) {
				if (checkTuple(data_block_ptr, i, table->getAttrTypeList(), cmpVec)) {
					data_block_ptr->SetTupleValue(i, attr_index, raw_value);
					if (compare(raw_value, data_block_ptr->GetDataPtr(0, attr_index), attr_type) < 0
						|| (next_block_ptr && compare(raw_value, next_block_ptr->GetDataPtr(0, attr_index), attr_type) >= 0)) {
						memcpy(buf, data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
						data_block_ptr->RemoveTuple(i);
						table->insertTuple(tuple);
						index_root = table->getIndexRoot();
					}
					updated_tuple_count++;
				}
			}
			if (data_block_ptr->RecordNum() != 0) {
				// insertion sort
				for (int i = 1; i < data_block_ptr->RecordNum(); i++) {
					memcpy(buf, data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
					int j = i - 1;
					for (; j >= 0 && ptr_compare(tuple[attr_index], data_block_ptr->GetDataPtr(j, attr_index), attr_type) < 0; j--) {
						memcpy(data_block_ptr->GetDataPtr(j + 1, 0), data_block_ptr->GetDataPtr(j, 0), data_block_ptr->tuple_size);
					}
					memcpy(data_block_ptr->GetDataPtr(j + 1, 0), buf, data_block_ptr->tuple_size);
				}
				// add index back
				index_root = index_manager_ptr->insertEntry(index_root, BPTree, data_block_ptr->GetDataPtr(0, attr_index), data_block_ptr->BlockIndex());
			}
			else {
				DeleteTableBlock(table, data_block_ptr);
			}
			buffer_manager->ReleaseBlock(data_block_ptr);
			if (next_block_ptr == NULL) break;
			else {
				data_block_ptr = next_block_ptr;
			}
		}
		if (index_root != table->getIndexRoot()) {
			catalog_manager.UpdateTablePrimaryIndex(tableName, index_root);
			table.updateDataBlockAddr(index_root);
		}
		delete [] buf;
		delete [] tuple;
	}
	delete [] raw_value;
	cout << updated_tuple_count << " Row Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

//
void ExeDelete(const std::string& tableName, const ComparisonVector& cmpVec)
{
	AutoPtr<Table> table(new Table(tableName));
	unsigned int deleted_tuple_count = 0;

	/* attribute check */
	int key_match = -1;
	for (unsigned int i = 0; i < cmpVec.size(); i++) {
		if (cmpVec[i].Comparand1.TypeName == "Attribute" &&
			table->getAttrName(table->getKeyIndex()) == cmpVec[i].Comparand1.Content) {
			key_match = i;
		}
	}

	// begin and end address for deletion traversal, here give the default value
	uint32_t begin_addr = table->getDataBlockAddr(), end_addr = 0;
	if (key_match != -1) {
		table->BlockFilter(cmpVec[key_match].Operation, raw_value, &begin_addr, &end_addr);
	}

	// delete in tuple scale
	uint32_t index_root = table->getIndexRoot();
	uint32_t next_block_addr = begin_addr
	while (next_block_addr != 0) {
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next_block_addr));
		data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
		int record_num = data_block_ptr->RecordNum();
		DBenum key_type = table->getAttrType(table->getKeyIndex);
		for (int i = record_num - 1; i >= 0; i--) {
			if (checkTuple(data_block_ptr, i, table->getAttrTypeList(), cmpVec)) {
				if (i == 0 && data_block_ptr->RecordNum() > 1) {
					AutoPtr<SearchResult> data_block_entry = index_manager_ptr->searchEntry(DB_BPTREE_INDEX, index_root, key_type,
						data_block_ptr->GetDataPtr(0, table->getKeyIndex()));
					index_root = index_manager_ptr->removeEntry(index_root, BPTree, data_block_entry);
					index_root = index_manager_ptr->insertEntry(index_root, BPTree,
						data_block_ptr->GetDataPtr(1, table->getKeyIndex()), data_block_ptr->BlockIndex());
					delete data_block_entry;
				}
				data_block_ptr->RemoveTuple(i);
				data_block_ptr->is_dirty = true;
				deleted_tuple_count += 1;
			}
		}
		next_block_addr = data_block_ptr->NextBlockIndex();
		if (data_block_ptr->RecordNum() == 0) {
			DeleteTableBlock(table, data_block_ptr);
		}
		buffer_manager->ReleaseBlock((Block* &)data_block_ptr);
	}
	if (index_root != table->getIndexRoot()) {
		catalog_manager.UpdateTablePrimaryIndex(tableName, index_root);
		table.updateDataBlockAddr(index_root);
	}
	cout << deleted_tuple_count << " Rows Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

void ExeDropIndex(const string& indexName)
{
	Catalog* catalog = &Catalog::Instance();
	try {
		catalog->DropIndex(indexName);
	}
	catch (const IndexNotFound &) {
		cout << "Index `" << indexName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	cout << "Drop Index Named `" << indexName << "` Successfully" << endl;
	cout << "end_result" << endl;
	Flush();
	return;
}

void ExeDropTable(const std::string& tableName, bool echo)
{
	Catalog* catalog = &Catalog::Instance();
	try {
		catalog->DropTable(tableName);
	}
	catch (const TableNotFound) {
		if (echo) cout << "Table `" << tableName << "` Not Found" << endl;
		if (echo) cout << "end_result" << endl;
		Flush();
		return;
	}
	if (echo) cout << "Drop Table `" << tableName << "` Successfully" << endl;
	if (echo) cout << "end_result" << endl;
	Flush();
	return;
}

void ExeCreateIndex(const std::string& tableName, const std::string& attrName, const string& indexName)

{
	Catalog* catalog = &Catalog::Instance();
	try {
		catalog->CreateIndex(indexName, tableName, attrName);
	}
	catch (const DuplicatedIndex &) {
		cout << "Duplicated Index `" << indexName << "`" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	catch (const TableNotFound) {
		cout << "Table `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	catch (const AttributeNotFound) {
		cout << "Attribute `" << attrName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
	}
	cout << "Create Index on `" << tableName << "` Successfully" << endl;
	cout << "end_result" << endl;
	Flush();
}

void ExeCreateTable(const std::string& tableName, const AttrDefinitionVector& defVec)
{
	Catalog* catalog = &Catalog::Instance();
	int attr_num = defVec.size();
	int key_index = -1;
	DBenum* attr_type_list = new DBenum[attr_num];
	string* attr_name_list = new string[attr_num];
	for (int i = 0; i < attr_num; i++) {
		attr_name_list[i] = defVec[i].AttrName;
		if (defVec[i].TypeName == "int") {
			attr_type_list[i] = DB_TYPE_INT;
		}
		else if (defVec[i].TypeName == "float") {
			attr_type_list[i] = DB_TYPE_FLOAT;
		}
		else if (defVec[i].TypeName == "char") {
			attr_type_list[i] = (DBenum)(DB_TYPE_CHAR + defVec[i].TypeParam);
		}
		if (defVec[i].bePrimaryKey) {
			key_index = i;
		}
	}
	try {
		catalog->CreateTable(tableName, attr_name_list, attr_type_list, attr_num, key_index);
	}
	catch (const DuplicatedTableName) {
		cout << "Table Named `" << tableName << "` Already Existed" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	cout << "Create Table `" << tableName << "` Successfully" << endl;
	cout << "end_result" << endl;
	Flush();
	return;
}