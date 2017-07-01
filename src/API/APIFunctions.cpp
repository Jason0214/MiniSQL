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
RecordBlock* insertTupleSafe(const void** tuple, TableMeta* tableMeta,  RecordBlock* dstBlock,BufferManager* bufferManager) {
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
	else if (operation == "<>"||operation=="!=") {
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
inline bool checkTuple(RecordBlock* block, int line, TableMeta* tableMeta,const ComparisonVector& sortedCmpVec) {
	const ComparisonVector& cmpVec = sortedCmpVec;
	for (int i = 0,j = 0;cmpVec.begin()+i < cmpVec.end(); i++) {
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
						result = typedCompare<int>(block->GetDataPtr(line, j),&integer,cmpOperator);
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

	//variables init
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	IndexManager* indexManager = nullptr;
	int indexPos;
	std::string operation = "";
	//make sure table name is valid
	std::string tableName;
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		std::cout << e.what() << std::endl;
		throw(e);
	}
	TableMeta* tableMeta;
	try{
		tableMeta = catalog->GetTableMeta(tableName);
	}
	catch(const TableNotFound &){
		cout << "Table `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	const void** tuple = (const void**)(new void*[tableMeta->attr_num]);
	RecordBlock* srcBlock;
	vector<Comparison> indexCmp;

	//create new temp table
	catalog->CreateTable(resultTableName, tableMeta->attr_name_list, tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->key_index);
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
		else {
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
		}
	}
	
	//primary index found
	if (isPrimary) {
		indexCmp.push_back(cmpVec[indexPos]);
		indexCmp[0].Operation = ">";
	}
	if (isPrimary&&(operation==">"||operation=="="||operation==">=")) {
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
			if (isPrimary&&(operation == "<" || operation == "=" || operation == "<=")) {
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
	if(indexRoot) bufferManager->ReleaseBlock((Block* &)indexRoot);
	if(indexManager) delete indexManager;
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
		std::cout << e.what() << std::endl;
		throw(e);
	}
	TableMeta* tableMeta;
	try{
		tableMeta = catalog->GetTableMeta(tableName);
	}
	catch(const TableNotFound &){
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
			dstBlock = insertTupleSafe(tuple, newTableMeta, dstBlock,bufferManager);
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
	std::string tableName1,tableName2;
	try {
		tableName1 = tableAlias.at(sourceTableName1);
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (exception& e) {
		std::cout << e.what() << std::endl;
		throw(e);
	}
	TableMeta* tableMeta1 = catalog->GetTableMeta(tableName1), *tableMeta2 = catalog->GetTableMeta(tableName2);
	//const void** tuple = (const void**)(new void*[tableMeta->attr_num]);
	RecordBlock* srcBlock1,*srcBlock2;
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
	for (int i = 0, j = 0, k = 0;i < tableMeta1->attr_num||j < tableMeta2->attr_num;k++) {
		if (i >= tableMeta1->attr_num||tableMeta1->GetAttrName(i) > tableMeta2->GetAttrName(j)) {
			newNameList[k] = tableMeta2->GetAttrName(j);
			newTypeList[k] = tableMeta2->GetAttrType(j);
			j++;
		}
		else if(j >= tableMeta2->attr_num || tableMeta1->GetAttrName(i) < tableMeta2->GetAttrName(j)) {
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
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (exception& e) {
		std::cout << e.what() << std::endl;
		throw(e);
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
		newNameList[i] = sourceTableName2 + "." + tableMeta2->GetAttrName(i- tableMeta1->attr_num);
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

//print out a table
void ExeOutputTable(const TableAliasMap& tableAlias, const string& sourceTableName)
{

	// if table on disk
	std::string tableName = sourceTableName;
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();

	TableMeta* tableMeta;
	try{
		tableMeta = catalog->GetTableMeta(tableName);
	}
	catch(const TableNotFound &){
		cout << "Table `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	unsigned short record_key = tableMeta->key_index < 0 ? 0 : tableMeta->key_index;	

	RecordBlock* result_block_ptr = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	result_block_ptr->Format(tableMeta->attr_type_list, tableMeta->attr_num, record_key);
#ifdef __PRETTY__
	//adjust align
	std::cout << std::left;
	//print out attr name
	int borderLen = tableMeta->attr_num - 1;
	for (int i = 0;i < tableMeta->attr_num;i++) {
		switch (tableMeta->attr_type_list[i]) {
		case DB_TYPE_INT: borderLen+=INTLEN;  break;
		case DB_TYPE_FLOAT: borderLen += FLOATLEN;  break;
		default:  borderLen += STRLEN;  break;
		}
	}
	std::string horizontalBorder(borderLen, '-');
	horizontalBorder = " " + horizontalBorder + " ";
	std::cout << horizontalBorder << std::endl;
	for (int i = 0;i < tableMeta->attr_num;i++) {
		std::cout << "|";
		switch (tableMeta->attr_type_list[i]) {
		case DB_TYPE_INT: std::cout << std::setw(INTLEN);  break;
		case DB_TYPE_FLOAT: std::cout << std::setw(FLOATLEN);;  break;
		default: std::cout << std::setw(STRLEN);;  break;
		}
		std::cout << tableMeta->GetAttrName(i);
	}
	std::cout << "|" << std::endl << horizontalBorder << std::endl;
	//print out data
	while(true){
		for(unsigned int i = 0; i < result_block_ptr->RecordNum(); i++){
			for(int j = 0; j < tableMeta->attr_num; j++){
				std::cout << "|";
				switch(tableMeta->attr_type_list[j]){
				case DB_TYPE_INT: std::cout << std::setw(INTLEN) << *(int*)result_block_ptr->GetDataPtr(i, j);  break;
				case DB_TYPE_FLOAT: std::cout << std::setw(FLOATLEN) << *(float*)result_block_ptr->GetDataPtr(i, j);  break;
				default: std::cout << std::setw(STRLEN) << (char*)result_block_ptr->GetDataPtr(i, j);  break;
				}
			}
			std::cout << "|";
			cout << endl;
		}
		uint32_t next = result_block_ptr->NextBlockIndex();
		if(next == 0){ 
			bufferManager->ReleaseBlock((Block* &)result_block_ptr);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)result_block_ptr);
		result_block_ptr =  dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}
	std::cout << horizontalBorder << std::endl;
	std::cout << "end_result" << std::endl;
	Flush();
#else
	//print out data
	while (true) {
		for (unsigned int i = 0; i < result_block_ptr->RecordNum(); i++) {
			for (int j = 0; j < tableMeta->attr_num; j++) {
				if(j) std::cout << "|";
				switch (tableMeta->attr_type_list[j]) {
				case DB_TYPE_INT: std::cout << *(int*)result_block_ptr->GetDataPtr(i, j);  break;
				case DB_TYPE_FLOAT: std::cout << *(float*)result_block_ptr->GetDataPtr(i, j);  break;
				default: std::cout << (char*)result_block_ptr->GetDataPtr(i, j);  break;
				}
			}
			cout << endl;
		}
		uint32_t next = result_block_ptr->NextBlockIndex();
		if (next == 0) {
			bufferManager->ReleaseBlock((Block* &)result_block_ptr);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)result_block_ptr);
		result_block_ptr = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
	}
#endif
	// if table is a temperary table not on disk
		//TODO
		
	//drop the srcTable if it's a temp table
	std::regex e("^_tmp.*");
	if (std::regex_match(tableName, e)) {
		ExeDropTable(tableName);
	}
}

void EndQuery()
{

}

int ptr_compare(const void* p1, const void* p2, DBenum type){
	switch(type){
		case DB_TYPE_INT:
			return *(int*) p1 - *(int*)p2;
			break;
		case DB_TYPE_FLOAT:
			return (int)(*(float*)p1 - *(float*)p2);
			break;
		default:
			return strcmp((char*)p1, (char*)p2);
			break;
	}
}

void InsertTuple(TableMeta* table_meta, const void** data_list)
{
	Catalog* catalog = &Catalog::Instance();
	BufferManager* buffer_manager = &BufferManager::Instance();
		
	uint32_t record_block_addr;
	/* do finding update index */
	IndexManager* index_manager = getIndexManager(table_meta->attr_type_list[table_meta->key_index]);
	Block* index_root = buffer_manager->GetBlock(table_meta->primary_index_addr);
	SearchResult* result_ptr = index_manager->searchEntry(index_root, BPTree, (void*)data_list[table_meta->key_index]);
	DBenum key_type = table_meta->attr_type_list[table_meta->key_index];
	void* B_plus_tree_key_ptr = NULL;
	if(result_ptr){
		if(ptr_compare((void *)((uint64_t)result_ptr->data + result_ptr->index*result_ptr->dataLen), 
								data_list[table_meta->key_index], key_type) == 0){
			if(table_meta->is_primary_key){
				cout << "Duplicated Primary Key" << endl;
				cout << "end_result" << endl;
				Flush();
				buffer_manager->ReleaseBlock(index_root);
				delete result_ptr;
				return;					
			}
			else{
				record_block_addr = *(result_ptr->ptrs + result_ptr->index);
			}
		}
		else{
			if(result_ptr->index == 0){
				record_block_addr = *(result_ptr->ptrs + result_ptr->index);
			}
			else{
				result_ptr->index--;
				record_block_addr = *(result_ptr->ptrs + result_ptr->index);		
			}
		}
	}
	else{
		record_block_addr = table_meta->table_addr;
	}	

	// do real insert
	RecordBlock* record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(record_block_addr));
	record_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);

	if(table_meta->is_primary_key){
		int i = record_block_ptr->FindTupleIndex(data_list[table_meta->key_index]);
		if(i >= 0 && ptr_compare(data_list[table_meta->key_index], record_block_ptr->GetDataPtr(i, table_meta->key_index), key_type) == 0){
			cout << "Duplicated Primary key" << endl;
			cout << "end_result" << endl;
			Flush();
			buffer_manager->ReleaseBlock(index_root);
			buffer_manager->ReleaseBlock((Block* &)record_block_ptr);
			delete result_ptr;
			delete index_manager;
			return;
		}
		record_block_ptr->InsertTupleByIndex(data_list, i>=0?i:0);
		}
	else{
		record_block_ptr->InsertTuple(data_list);
	}

	// update index
	if(!result_ptr){
		index_manager->insertEntry(index_root, BPTree, record_block_ptr->GetDataPtr(0, table_meta->key_index), record_block_addr);
	}
	else{
		index_manager->removeEntry(index_root, BPTree, result_ptr);
		index_manager->insertEntry(index_root, BPTree, record_block_ptr->GetDataPtr(0, table_meta->key_index), record_block_addr);
	}
	//check space, split if needed
	if(!record_block_ptr->CheckEmptySpace()){
		RecordBlock* new_block_ptr = catalog->SplitRecordBlock(record_block_ptr, table_meta->attr_type_list, 
												table_meta->attr_num, table_meta->key_index);
		index_manager->insertEntry(index_root, BPTree, new_block_ptr->GetDataPtr(0, table_meta->key_index), new_block_ptr->BlockIndex());
		buffer_manager->ReleaseBlock((Block* &)new_block_ptr);
	}
	// update index root
	if(index_root->BlockIndex() != table_meta->primary_index_addr){
		catalog->UpdateTablePrimaryIndex(table_meta->table_name, index_root->BlockIndex());
	}
	record_block_ptr->is_dirty = true;
	buffer_manager->ReleaseBlock((Block* &)record_block_ptr);
	buffer_manager->ReleaseBlock(index_root);
	delete index_manager;
	delete result_ptr;
	cout << "1 Row Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

void ExeInsert(const std::string& tableName, InsertValueVector& values){
	Catalog* catalog_manager = &Catalog::Instance();
	TableMeta* table_meta = NULL;
	try{
		table_meta = catalog_manager->GetTableMeta(tableName);
	}
	catch(const TableNotFound &){
		cout << "Table `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	int temp_int_buf[32];
	int temp_int_pointer = 0;
	float temp_float_buf[32];
	int temp_float_pointer = 0;
	int temp_int;
	float temp_float;
	if(table_meta->attr_num != values.size()){
		cout << "Attributes Number Unmatch" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	bool error = false;
	const void** data_list = new const void*[table_meta->attr_num];
	for(int i = 0; !error && i < table_meta->attr_num; i++){
		stringstream ss(values[i]);
		ss.exceptions(std::ios::failbit);
		switch(table_meta->attr_type_list[i]){
			case DB_TYPE_INT:
				ss >> temp_int;
				temp_int_buf[temp_int_pointer++] = temp_int;
				data_list[i] = &temp_int_buf[temp_int_pointer-1];
				break;
			case DB_TYPE_FLOAT:
				ss >> temp_float;
				temp_float_buf[temp_float_pointer++] = temp_float;
				data_list[i] = &temp_float_buf[temp_float_pointer -1];
				break;
			default:
				if(table_meta->attr_type_list[i] - DB_TYPE_CHAR < (int)values[i].length()){
					error = true;
					break;
				}
				data_list[i] = values[i].c_str();
				break;
		}
	}
	if(error){
		cout << "Attributes Types Not Satisfied" << endl;
		cout << "end_result" << endl;
		Flush();
	}
	else{
		InsertTuple(table_meta, data_list);
	}
	delete table_meta;
	delete data_list;
}

TraversalAddr FindAddr(TableMeta* table_meta, int primary_tag, const ComparisonVector& cmpVec){
	TraversalAddr ret;
	ret.begin = table_meta->table_addr;
	ret.end = 0;
	
	BufferManager* buffer_manager = &BufferManager::Instance();
	/* use primary index to find the target block */
	IndexManager* index_manager_ptr = getIndexManager(table_meta->attr_type_list[table_meta->key_index]);
	Block* index_root = buffer_manager->GetBlock(table_meta->primary_index_addr);

	// temp var
	int temp_int;
	float temp_float;
	void* temp_ptr;
	stringstream ss(cmpVec[primary_tag].Comparand2.Content);
	ss.exceptions(std::ios::failbit);
	switch(table_meta->attr_type_list[table_meta->key_index]){
		case DB_TYPE_INT:
			ss >> temp_int;
			temp_ptr = &temp_int;
			break;
		case DB_TYPE_FLOAT:
			ss >> temp_float;
			temp_ptr = &temp_float;
			break;
		default:
			temp_ptr = (void*)(cmpVec[primary_tag].Comparand2.Content.c_str());
			break;
	}

	uint32_t target_block_addr;
	SearchResult* result_ptr = index_manager_ptr->searchEntry(index_root, BPTree, temp_ptr);
	if(ptr_compare((void*)((uint64_t)result_ptr->data + result_ptr->index*result_ptr->dataLen), temp_ptr
			, table_meta->attr_type_list[table_meta->key_index]) == 0){
		target_block_addr = *(result_ptr->ptrs + result_ptr->index);
	}
	else{
		if(result_ptr->index == 0){
			target_block_addr = *(result_ptr->ptrs + result_ptr->index);
		}
		else{
			target_block_addr = *(result_ptr->ptrs + result_ptr->index - 1);		
		}
	}
	delete result_ptr;
	delete index_manager_ptr;
	buffer_manager->ReleaseBlock(index_root);
	/* deletion starts here */
	RecordBlock* target_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_addr));
	target_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
	
	// find the begin and end of tuple traversal
	if((cmpVec[primary_tag].Operation == "<")){
		ret.end = target_block_ptr->NextBlockIndex();
	}
	else if(cmpVec[primary_tag].Operation == "<=" && target_block_ptr->NextBlockIndex() != 0){
		RecordBlock* next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_ptr->NextBlockIndex()));
		while(true){
			next_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			if(ptr_compare(next_block_ptr->GetDataPtr(0, table_meta->key_index), temp_ptr, table_meta->attr_type_list[table_meta->key_index]) != 0){
				ret.end = next_block_ptr->BlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				break;
			}
			else{
				uint32_t next = next_block_ptr->NextBlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				if(next == 0) break;
				else{
					next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next));
				}
			}
		}
	}
	else if(cmpVec[primary_tag].Operation == ">="){
		ret.begin = target_block_ptr->BlockIndex();
	}
	else if(cmpVec[primary_tag].Operation == ">" && target_block_ptr->NextBlockIndex() != 0){
		RecordBlock* next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_ptr->NextBlockIndex()));
		while(true){
			next_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			if(ptr_compare(next_block_ptr->GetDataPtr(0, table_meta->key_index), temp_ptr, table_meta->attr_type_list[table_meta->key_index]) != 0){
				ret.begin = next_block_ptr->PreBlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				break;
			}
			else{
				uint32_t next = next_block_ptr->NextBlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				if(next == 0) break;
				else{
					next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next));
				}
			}
		}
	}
	else if(cmpVec[primary_tag].Operation == "=" && target_block_ptr->NextBlockIndex() != 0){
		ret.begin = target_block_ptr->BlockIndex();
		RecordBlock* next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_ptr->NextBlockIndex()));	
		while(true){
			next_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			if(ptr_compare(next_block_ptr->GetDataPtr(0, table_meta->key_index), temp_ptr, table_meta->attr_type_list[table_meta->key_index]) != 0){
				ret.begin = next_block_ptr->PreBlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				break;
			}
			else{
				uint32_t next = next_block_ptr->NextBlockIndex();
				buffer_manager->ReleaseBlock((Block* &)next_block_ptr);
				if(next == 0) break;
				else{
					next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next));
				}
			}
		}			
	}
	buffer_manager->ReleaseBlock((Block* &)target_block_ptr);
	return ret;
}

void ExeUpdate(const std::string& tableName, const std::string& attrName, 
	const std::string& value, const ComparisonVector& cmpVec)
{
	BufferManager* buffer_manager = &BufferManager::Instance();
	Catalog* catalog_manager = &Catalog::Instance();
	TableMeta* table_meta = NULL;
	int updated_tuple_count = 0;
	try{
		table_meta = catalog_manager->GetTableMeta(tableName);
	}
	catch(const TableNotFound &){
		cout << "Table Name `" << tableName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}
	// find attribute index and make sure it exists in the tab;e
	int attr_index = -1;
	for(int i = 0; i < table_meta->attr_num; i++){
		if(table_meta->attr_name_list[i] == attrName){
			attr_index = i;
			break;
		}
	}
	if(attr_index == -1){
		delete table_meta;
		cout << "Attribute Name `" << attrName << "` Not Found" << endl;
		cout << "end_result" << endl;
		Flush();
		return;
	}

	// temp var to transform `value` to const void*
	int temp_int;
	float temp_float;
	void* temp_ptr;
	stringstream ss(value);
	ss.exceptions(std::ios::failbit);
	switch (table_meta->attr_type_list[attr_index]) {
	case DB_TYPE_INT:
		ss >> temp_int;
		temp_ptr = &temp_int;
		break;
	case DB_TYPE_FLOAT:
		ss >> temp_float;
		temp_ptr = &temp_float;
		break;
	default:
		temp_ptr = (void*)(value.c_str());
		break;
	}

	// test if primary index exists in the cmpVec
	int primary_tag = -1;
	for (unsigned int i = 0; i < cmpVec.size(); i++) {
		if (cmpVec[i].Comparand1.TypeName == "Attribute" &&
			table_meta->attr_name_list[table_meta->key_index] == cmpVec[i].Comparand1.Content) {
			primary_tag = i;
		}
	}
	// begin and end address for deletion traversal, here give the default value
	uint32_t begin_addr = table_meta->table_addr;
	uint32_t end_addr = 0;
	if (primary_tag != -1) {
		TraversalAddr addr_struct = FindAddr(table_meta, primary_tag, cmpVec);
		begin_addr = addr_struct.begin;
		end_addr = addr_struct.end;
	}

	DBenum attr_type = table_meta->attr_type_list[attr_index];
	if(attr_index != table_meta->key_index){
		// no need to update primary index
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(begin_addr));
		while (true) {
			data_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			int record_num = data_block_ptr->RecordNum();
			for (int i = 0; i < record_num; i++) {
				if (checkTuple(data_block_ptr, i, table_meta, cmpVec)) {
					data_block_ptr->SetTupleValue(i, attr_index, temp_ptr);
					updated_tuple_count++;
				}
			}
			uint32_t next = data_block_ptr->NextBlockIndex();
			buffer_manager->ReleaseBlock((Block* &)data_block_ptr);
			if (next == end_addr) break;
			else {
				data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next));
			}
		}
	}
	else{
		// if attr_index is same as primary key, we should update index
		IndexManager* index_manager_ptr = getIndexManager(attr_type);
		Block* index_root = buffer_manager->GetBlock(table_meta->primary_index_addr);

		//pre-check whether the new attribute value would cause duplicated primary key
		if (table_meta->is_primary_key) {
			uint32_t target_block_addr;
			SearchResult* block_entry = index_manager_ptr->searchEntry(index_root, BPTree, temp_ptr);
			if (ptr_compare((void*)((uint64_t)block_entry->data + block_entry->dataLen * block_entry->index), temp_ptr, attr_type) == 0) {
				target_block_addr = *(block_entry->ptrs + block_entry->index);
			}
			else {
				if (block_entry->index == 0) {
					target_block_addr = *(block_entry->ptrs + block_entry->index);
				}
				else {
					target_block_addr = *(block_entry->ptrs + block_entry->index - 1);
				}
			}
			delete block_entry;
			RecordBlock* target_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(target_block_addr));
			target_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			int target_index = target_block_ptr->FindTupleIndex(temp_ptr);
			if (target_index >= 0 && ptr_compare(temp_ptr, target_block_ptr->GetDataPtr(target_index, table_meta->key_index), attr_type) == 0) {
				cout << "Duplicated Primary Key" << endl;
				cout << "end_result" << endl;
				Flush();
				return;
			}
		}
		
	/* update begins here */
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(begin_addr));
		char* buf = new char[data_block_ptr->tuple_size];
		const void ** data_list = new const void*[table_meta->attr_num];
		for(int i = 0; i < table_meta->attr_num; i++){
			data_list[i] = buf + (data_block_ptr->GetDataPtr(0, i) - data_block_ptr->GetDataPtr(0, 0));
		}
		data_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
		while (true) {
			RecordBlock* next_block_ptr = NULL;
			if (data_block_ptr->NextBlockIndex() != 0) {
				next_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(data_block_ptr->NextBlockIndex()));
				next_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
			}
			int record_num = data_block_ptr->RecordNum();
			// tempararily remove the index
			SearchResult* data_block_entry = index_manager_ptr->searchEntry(index_root, BPTree, data_block_ptr->GetDataPtr(0, attr_index));
			index_root = index_manager_ptr->removeEntry(index_root, BPTree, data_block_entry);
			delete data_block_entry;

			for (int i = record_num - 1; i >= 0; i--) {
				if (checkTuple(data_block_ptr, i, table_meta, cmpVec)) {
					data_block_ptr->SetTupleValue(i, attr_index, temp_ptr);
					if (ptr_compare(temp_ptr, data_block_ptr->GetDataPtr(0, attr_index), attr_type) < 0
						|| (next_block_ptr && ptr_compare(temp_ptr, next_block_ptr->GetDataPtr(0, attr_index), attr_type) >= 0)) {
						memcpy(buf, data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
						data_block_ptr->RemoveTuple(i);
						InsertTuple(table_meta, data_list);
					}
					updated_tuple_count++;
				}
			}
			if (data_block_ptr->RecordNum() != 0) {
				// insertion sort
				for (int i = 1; i < data_block_ptr->RecordNum(); i++) {
					memcpy(buf, data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
					int j = i - 1;
					for (; j >= 0 && ptr_compare(data_list[attr_index], data_block_ptr->GetDataPtr(j, attr_index), attr_type) < 0; j--) {
						memcpy(data_block_ptr->GetDataPtr(j + 1, 0), data_block_ptr->GetDataPtr(j, 0), data_block_ptr->tuple_size);
					}
					memcpy(data_block_ptr->GetDataPtr(j + 1, 0), buf, data_block_ptr->tuple_size);
				}
				// add index back
				index_root = index_manager_ptr->insertEntry(index_root, BPTree, data_block_ptr->GetDataPtr(0, attr_index), data_block_ptr->BlockIndex());
			}
			else {
				// if only one record remained in the block
				if (data_block_ptr->PreBlockIndex() == 0 && data_block_ptr->NextBlockIndex() == 0) {}
				else if (data_block_ptr->PreBlockIndex() == 0) {
					catalog_manager->UpdateTableDataAddr(tableName, data_block_ptr->NextBlockIndex());
					table_meta->table_addr = data_block_ptr->NextBlockIndex();
					Block* next_block_ptr = buffer_manager->GetBlock(data_block_ptr->NextBlockIndex());
					next_block_ptr->PreBlockIndex() = 0;
					next_block_ptr->is_dirty = true;
					buffer_manager->ReleaseBlock(next_block_ptr);
					buffer_manager->DeleteBlock((Block* &)data_block_ptr);
				}
				else if (data_block_ptr->NextBlockIndex() == 0) {
					Block* pre_block_ptr = buffer_manager->GetBlock(data_block_ptr->PreBlockIndex());
					pre_block_ptr->NextBlockIndex() = 0;
					pre_block_ptr->is_dirty = true;
					buffer_manager->ReleaseBlock(pre_block_ptr);
					buffer_manager->DeleteBlock((Block* &)data_block_ptr);
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
					buffer_manager->DeleteBlock((Block* &)data_block_ptr);
				}
			}
			buffer_manager->ReleaseBlock((Block* &)data_block_ptr);
			if (next_block_ptr == NULL) break;
			else {
				data_block_ptr = next_block_ptr;
			}
		}
		if (index_root->BlockIndex() != table_meta->primary_index_addr) {
			catalog_manager->UpdateTablePrimaryIndex(tableName, index_root->BlockIndex());
		}
		buffer_manager->ReleaseBlock(index_root);
		delete buf;
		delete data_list;
		delete index_manager_ptr;
	}
	delete table_meta;
	cout << updated_tuple_count << " Row Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

//
void ExeDelete(const std::string& tableName, const ComparisonVector& cmpVec)
{

	Catalog* catalog = &Catalog::Instance();
	BufferManager* buffer_manager = &BufferManager::Instance();
	TableMeta* table_meta = catalog->GetTableMeta(tableName);
	IndexManager* index_manager_ptr = getIndexManager(table_meta->attr_type_list[table_meta->key_index]);
	Block* index_root = buffer_manager->GetBlock(table_meta->primary_index_addr);


	unsigned int deleted_tuple_count = 0;
/* attribute check */
	int primary_tag = -1;
	for(unsigned int i = 0; i < cmpVec.size(); i++){
		if(cmpVec[i].Comparand1.TypeName == "Attribute" &&
		table_meta->attr_name_list[table_meta->key_index] == cmpVec[i].Comparand1.Content){
			primary_tag = i;
		}
	}

	// begin and end address for deletion traversal, here give the default value
	uint32_t begin_addr = table_meta->table_addr;
	uint32_t end_addr = 0;
	if(primary_tag != -1){
		TraversalAddr addr_struct = FindAddr(table_meta, primary_tag, cmpVec);
		begin_addr = addr_struct.begin;
		end_addr = addr_struct.end;
	}

	// delete in tuple scale
	RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(begin_addr));
	while(true){
		data_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
		int record_num = data_block_ptr->RecordNum();
		for(int i = record_num - 1; i >= 0; i--){
			if(checkTuple(data_block_ptr, i, table_meta, cmpVec)){
				if(i == 0 && data_block_ptr->RecordNum() > 1){
					SearchResult* data_block_entry = index_manager_ptr->searchEntry(index_root, BPTree, 
													data_block_ptr->GetDataPtr(0, table_meta->key_index));
					index_root = index_manager_ptr->removeEntry(index_root, BPTree, data_block_entry);
					index_root = index_manager_ptr->insertEntry(index_root, BPTree, 
										data_block_ptr->GetDataPtr(1, table_meta->key_index),data_block_ptr->BlockIndex());
					delete data_block_entry;
				}
				data_block_ptr->RemoveTuple(i);
				data_block_ptr->is_dirty = true;
				deleted_tuple_count += 1;
			}
		}
		if(data_block_ptr->RecordNum() == 0){
			// if only one record remained in the block
			if(data_block_ptr->PreBlockIndex() == 0 && data_block_ptr->NextBlockIndex() == 0){}
			else if(data_block_ptr->PreBlockIndex() == 0){
				catalog->UpdateTableDataAddr(tableName, data_block_ptr->NextBlockIndex());
				table_meta->table_addr = data_block_ptr->NextBlockIndex();
				Block* next_block_ptr = buffer_manager->GetBlock(data_block_ptr->NextBlockIndex());
				next_block_ptr->PreBlockIndex() = 0;
				next_block_ptr->is_dirty = true;
				buffer_manager->ReleaseBlock(next_block_ptr);
				buffer_manager->DeleteBlock((Block* &)data_block_ptr);
			}
			else if(data_block_ptr->NextBlockIndex() == 0){
				Block* pre_block_ptr = buffer_manager->GetBlock(data_block_ptr->PreBlockIndex());
				pre_block_ptr->NextBlockIndex() = 0;
				pre_block_ptr->is_dirty = true;
				buffer_manager->ReleaseBlock(pre_block_ptr);
				buffer_manager->DeleteBlock((Block* &)data_block_ptr);
			}
			else{
				Block* pre_block_ptr = buffer_manager->GetBlock(data_block_ptr->PreBlockIndex());
				Block* next_block_ptr = buffer_manager->GetBlock(data_block_ptr->NextBlockIndex());
				pre_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
				next_block_ptr->PreBlockIndex() = pre_block_ptr->BlockIndex();
				pre_block_ptr->is_dirty = true;
				next_block_ptr->is_dirty = true;
				buffer_manager->ReleaseBlock(next_block_ptr);
				buffer_manager->ReleaseBlock(pre_block_ptr);
				buffer_manager->DeleteBlock((Block* &)data_block_ptr);
			}
		}
		uint32_t next = data_block_ptr->NextBlockIndex();
		buffer_manager->ReleaseBlock((Block* &)data_block_ptr);
		if(next == end_addr){
			break;
		}
		else{
			data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager->GetBlock(next));
		}
	}
	buffer_manager->ReleaseBlock(index_root);
	delete table_meta;
	delete index_manager_ptr;
	cout << deleted_tuple_count << " Rows Affected" << endl;
	cout << "end_result" << endl;
	Flush();
}

void ExeDropIndex(const std::string& tableName, const std::string& indexName)
{
	Catalog* catalog = &Catalog::Instance();
	try{
		catalog->DropIndex(indexName);
	}
	catch(const IndexNotFound &){
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
	try{
		catalog->DropTable(tableName);
	}
	catch (const TableNotFound){
		if (echo) cout << "Table `" << tableName << "` Not Found" << endl;
		if (echo) cout << "end_result" << endl;
		Flush();
		return ;
	}
	if (echo) cout << "Drop Table `" << tableName << "` Successfully" << endl;
	if (echo) cout << "end_result" << endl;
	Flush();
	return;
}

void ExeCreateIndex(const std::string& tableName, const std::string& attrName, const string& indexName)

{
	Catalog* catalog = &Catalog::Instance();
	try{
		catalog->CreateIndex(indexName, tableName, attrName);
	}
	catch(const DuplicatedIndexName &){
		cout << "Duplicated Index Name `" << indexName << "`" << endl;
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
	for(int i = 0; i < attr_num; i++){
		attr_name_list[i] = defVec[i].AttrName;
		if(defVec[i].TypeName == "int"){
			attr_type_list[i] = DB_TYPE_INT;
		}
		else if(defVec[i].TypeName == "float"){
			attr_type_list[i] = DB_TYPE_FLOAT;
		}
		else if(defVec[i].TypeName == "char"){
			attr_type_list[i] = (DBenum)(DB_TYPE_CHAR + defVec[i].TypeParam);
		}
		if(defVec[i].bePrimaryKey){
			key_index = i;
		}
	}
	try{
		catalog->CreateTable(tableName, attr_name_list, attr_type_list, attr_num, key_index);
	}
	catch (const DuplicatedTableName){
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