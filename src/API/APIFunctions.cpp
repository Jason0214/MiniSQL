
#include "APIStructures.h"
#include "APIFunctions.h"
#include "IO.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include "../Type/ConstChar.h"
#include <string>
#include <sstream>

//may add to record manager
//get index manager according to type
IndexManager* getIndexManager(DBenum type) {
	IndexManager* indexManager;
	if (type == DB_TYPE_INT) {
		indexManager = new TypedIndexManager<int>();
	}
	else if (type == DB_TYPE_FLOAT) {
		indexManager = new TypedIndexManager<float>();
	}
	else if (type - DB_TYPE_CHAR < 16) {
		indexManager = new TypedIndexManager<ConstChar<16>>();
	}
	else if (type - DB_TYPE_CHAR < 33) {
		indexManager = new TypedIndexManager<ConstChar<33>>();
	}
	else if (type - DB_TYPE_CHAR < 64) {
		indexManager = new TypedIndexManager<ConstChar<64>>();
	}
	else if (type - DB_TYPE_CHAR < 128) {
		indexManager = new TypedIndexManager<ConstChar<128>>();
	}
	else {
		indexManager = new TypedIndexManager<ConstChar<256>>();
	}
	return indexManager;
}

//may add to record manager
RecordBlock* insertTupleSafe(const void** tuple, TableMeta* tableMeta,  RecordBlock* dstBlock,BufferManager* bufferManager) {
	if (!dstBlock->CheckEmptySpace()) {
		RecordBlock* newBlock = dynamic_cast<RecordBlock*>(bufferManager->CreateBlock(DB_RECORD_BLOCK));
		dstBlock->NextBlockIndex() = newBlock->BlockIndex();
		bufferManager->ReleaseBlock((Block*&)(dstBlock));
		dstBlock = newBlock;
		dstBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);
	}
	dstBlock->InsertTuple(tuple);
	return dstBlock;
}
//may add to record manager
//compare template function
template <class T>
inline bool compare(const void* a, const void* b, const std::string &operation) {
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
//may add to record manager
//check if a tuple is valid
//cmpVec is sorted
inline bool checkTuple(RecordBlock* block, int line,TableMeta* tableMeta,const ComparisonVector& sortedCmpVec) {
	const ComparisonVector& cmpVec = sortedCmpVec;
	std::string cmpOperator;
	for (int i = 0,j = 0;cmpVec.begin()+i < cmpVec.end(); i++) {
		std::string cmpOperator = cmpVec[i].Operation;
		for (; j < tableMeta->attr_num; j++) {
			bool result=true;
			if (cmpVec[i].Comparand1.Content == tableMeta->GetAttrName(j)) {
				stringstream ss(cmpVec[i].Comparand2.Content);
				DBenum type = tableMeta->attr_type_list[j];
				switch (type) {
					case DB_TYPE_INT:
						int integer;
						ss >> integer;
						result = compare<int>(block->GetDataPtr(line, j),&integer,cmpOperator);
						break;
					case DB_TYPE_FLOAT:
						float num;
						ss >> num;
						result = compare<float>(block->GetDataPtr(line, j), &num, cmpOperator);
						break;
					default:
						if (type - DB_TYPE_CHAR < 16) {
							ConstChar<16> str;
							ss >> str;
							result = compare<ConstChar<16>>(block->GetDataPtr(line, j), &str, cmpOperator);
						}
						else if (type - DB_TYPE_CHAR < 33) {
							ConstChar<33> str;
							ss >> str;
							result = compare<ConstChar<33>>(block->GetDataPtr(line, j), &str, cmpOperator);
						}
						else if (type - DB_TYPE_CHAR < 64) {
							ConstChar<64> str;
							ss >> str;
							result = compare<ConstChar<64>>(block->GetDataPtr(line, j), &str, cmpOperator);
						}
						else if (type - DB_TYPE_CHAR < 128) {
							ConstChar<128> str;
							ss >> str;
							result = compare<ConstChar<128>>(block->GetDataPtr(line, j), &str, cmpOperator);
						}
						else {
							ConstChar<256> str;
							ss >> str;
							result = compare<ConstChar<256>>(block->GetDataPtr(line, j), &str, cmpOperator);
						}
						break;
				}
				if (!result) return result;
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
	IndexManager* indexManager;
	int indexPos;
	std::string operation = "";
	//make sure table name is valid
	std::string tableName;
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		throw(e);
	}
	TableMeta* tableMeta = catalog->GetTableMeta(tableName);
	const void** tuple = (const void**)(new void*[tableMeta->attr_num]);
	RecordBlock* srcBlock;
	vector<Comparison> indexCmp;

	//create new temp table
	catalog->CreateTable(resultTableName, tableMeta->attr_name_list, tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);

	//try to find a proper index
	Block* indexRoot = nullptr;
	DBenum indexType;
	std::string indexContent;
	bool isPrimary = false; //if it is a primary index
	std::string primaryKeyName = (tableMeta->primay_key_index >= 0) ?
		tableMeta->GetAttrName(tableMeta->primay_key_index) : "";
	for (auto i = cmpVec.begin();i < cmpVec.end();i++) {
		//check if the attribute is a primary index
		if (primaryKeyName == (*i).Comparand1.Content) {
			indexRoot = bufferManager->GetBlock(catalog->GetIndex(tableName,tableMeta->primay_key_index));
			indexType = tableMeta->GetAttrType(tableMeta->primary_index_addr);
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
			char str[]=indexContent.c_str;
			pos = indexManager->searchEntry(indexRoot, BPTree, str);
			break;
		}
		ptr = static_cast<BPlusNode<float>*>(pos->node)->ptrs()[pos->index + 1];
		srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(ptr));
		delete pos;
	}
	//no primary index found
	else {
		srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	}
	while (true) {
		srcBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);
		for (int i = 0; i < srcBlock->RecordNum(); i++) {
			//if the current key is already larger than the searched primary index
			if (isPrimary&&(operation == "<" || operation == "=" || operation == "<=")) {
				if (checkTuple(srcBlock, i, tableMeta, indexCmp)) {
					bufferManager->ReleaseBlock((Block* &)srcBlock);
					break;
				}
			}
			//if the tuple fit the comparisonVector
			if (checkTuple(srcBlock, i, tableMeta, cmpVec)) {
				for (int j = 0;j < tableMeta->attr_num;j++) {
					tuple[j] = srcBlock->GetDataPtr(i, j);
				}
				//safe insert
				dstBlock = insertTupleSafe(tuple, tableMeta, srcBlock, bufferManager);
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
	delete indexManager;
	delete bufferManager;
	delete catalog;
	delete tableMeta;
	delete[] tuple;
}


//attrVec is sorted
void ExeProject(TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec)
{
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	Block* block = bufferManager->GetBlock(tableMeta->table_addr);
	std::vector<int> attrIndexVec;
	//get attr index
	for (int i = 0, j = 0;i < tableMeta->attr_num&&j < attrVec.size();i++) {
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
	for (int i = 0;i < attrIndexVec.size();i++) {
		newNameList[i] = tableMeta->GetAttrName(attrIndexVec[i]);
		newTypeList[i] = tableMeta->GetAttrType(attrIndexVec[i]);
	}
	int keyIndex = 0;
	catalog->CreateTable(resultTableName, newNameList, newTypeList, tableMeta->attr_num, keyIndex);
	RecordBlock* dstBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(catalog->GetTableMeta(resultTableName)->table_addr));
	dstBlock->is_dirty = true;
	dstBlock->Format(newTypeList, attrIndexVec.size(), keyIndex);
	TableMeta* newTableMeta = catalog->GetTableMeta(resultTableName);

	//insert data into new table
	RecordBlock* srcBlock = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	const void** tuple = (const void**)(new void*[attrIndexVec.size()]);
	while (true) {
		srcBlock->Format(tableMeta->attr_type_list, tableMeta->attr_num, tableMeta->primay_key_index);
		for (int i = 0; i < srcBlock->RecordNum(); i++) {
			for (int j = 0;j < attrIndexVec.size();j++) {
				tuple[j] = srcBlock->GetDataPtr(i, attrIndexVec[j]);
			}
			//safe insert
			dstBlock = insertTupleSafe(tuple, newTableMeta, srcBlock,bufferManager);
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
	delete bufferManager;
	delete catalog;
	delete tableMeta;
	delete[] tuple;
	delete[] newNameList;
	delete[] newTypeList;
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
		throw(e);
	}
	TableMeta* tableMeta1 = catalog->GetTableMeta(tableName1), *tableMeta2 = catalog->GetTableMeta(tableName2);
	//const void** tuple = (const void**)(new void*[tableMeta->attr_num]);
	RecordBlock* srcBlock1,*srcBlock2;
	std::vector<Comparison> indexCmp;
	std::vector<int> commonAttrIndex1, commonAttrIndex2;

	//get common attrs
	for (int i = 0, j = 0;i < tableMeta1->attr_num&&j < tableMeta2->attr_num;) {
		if (tableMeta1->GetAttrName(i) > tableMeta2->GetAttrName(j)) j++;
		else if (tableMeta1->GetAttrName(i) < tableMeta2->GetAttrName(j)) i++;
		else {
			commonAttrIndex1.push_back(i);
			commonAttrIndex2.push_back(j);
		}
	}

	//create new temp table
	std::string *newNameList;
	DBenum *newTypeList;
	int newListSize = tableMeta1->attr_num + tableMeta2->attr_num - commonAttrIndex1.size();
	const void** tuple = (const void**)(new void*[newListSize]);
	newNameList = new std::string[newListSize];
	newTypeList = new DBenum[newListSize];
	for (int i = 0, j = 0, k = 0;i < tableMeta1->attr_num&&j < tableMeta2->attr_num;k++) {
		if (tableMeta1->GetAttrName(i) > tableMeta2->GetAttrName(j)) {
			j++;
			newNameList[k] = tableMeta2->GetAttrName(j);
			newTypeList[k] = tableMeta2->GetAttrType(j);
		}
		else {
			i++;
			newNameList[k] = tableMeta2->GetAttrName(i);
			newTypeList[k] = tableMeta2->GetAttrType(i);
		}
		if (tableMeta1->GetAttrName(i) == tableMeta2->GetAttrName(j)) j++;
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
		srcBlock1->Format(tableMeta1->attr_type_list, tableMeta1->attr_num, tableMeta1->primay_key_index);
		for (int i = 0; i < srcBlock1->RecordNum(); i++) {
			//second loop:srcBlock2
			while (true) {
				srcBlock2->Format(tableMeta2->attr_type_list, tableMeta2->attr_num, tableMeta2->primay_key_index);
				for (int j = 0; j < srcBlock1->RecordNum(); j++) {
					bool result = true; //compare result
					for (int k = 0;k < commonAttrIndex1.size();k++) {
						DBenum type = tableMeta1->attr_type_list[commonAttrIndex1[k]];
						switch (type) {
						case DB_TYPE_INT:
							result = compare<int>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]), 
								srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							break;
						case DB_TYPE_FLOAT:
							result = compare<float>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
								srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							break;
						default:
							if (type - DB_TYPE_CHAR < 16) {
								result = compare<ConstChar<16>>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
									srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							}
							else if (type - DB_TYPE_CHAR < 33) {
								result = compare<ConstChar<33>>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
									srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							}
							else if (type - DB_TYPE_CHAR < 64) {
								result = compare<ConstChar<64>>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
									srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							}
							else if (type - DB_TYPE_CHAR < 128) {
								result = compare<ConstChar<128>>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
									srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							}
							else {
								result = compare<ConstChar<256>>(srcBlock1->GetDataPtr(i, commonAttrIndex1[k]),
									srcBlock1->GetDataPtr(j, commonAttrIndex2[k]), cmpOperator);
							}
							break;
						}
						if (!result) break;
					}
					//join two tuples
					if (result) {
						for (int ii = 0,jj=0;ii < tableMeta1->attr_num;ii++) {
							for (;jj < newTableMeta->attr_num;jj++) {
								if (tableMeta1->GetAttrName[ii] == newTableMeta->GetAttrName[jj]) {
									tuple[jj] = srcBlock1->GetDataPtr(i, ii);
									break;
								}
							}
						}
						for (int ii = 0, jj = 0;ii < tableMeta2->attr_num;ii++) {
							for (;jj < newTableMeta->attr_num;jj++) {
								if (tableMeta2->GetAttrName[ii] == newTableMeta->GetAttrName[jj]) {
									tuple[jj] = srcBlock2->GetDataPtr(j, ii);
									break;
								}
							}
						}
					}
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
	delete bufferManager;
	delete catalog;
	delete tableMeta1;
	delete tableMeta2;
	delete newTableMeta;
	delete[] tuple;
	delete[] newNameList;
	delete[] newTypeList;
}

void ExeCartesian(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{

}

void ExeOutputTable(TableAliasMap& tableAlias, const string& sourceTableName)
{
	// Print "end_result" in the last line to stop


	// if table on disk
	Catalog* catalog = &Catalog::Instance();
	BufferManager* bufferManager = &BufferManager::Instance();
	TableMeta* tableMeta = catalog->GetTableMeta(tableAlias[sourceTableName]);
	unsigned short record_key = tableMeta->primay_key_index < 0 ? 0 : tableMeta->primay_key_index;	

	RecordBlock* result_block_ptr = dynamic_cast<RecordBlock*>(bufferManager->GetBlock(tableMeta->table_addr));
	result_block_ptr->Format(tableMeta->attr_type_list, tableMeta->attr_num, record_key);
	while(true){
		for(unsigned int i = 0; i < result_block_ptr->RecordNum(); i++){
			for(unsigned int j = 0; j < tableMeta->attr_num; j++){
				switch(tableMeta->attr_type_list[j]){
					case DB_TYPE_INT: cout <<  *(int*)result_block_ptr->GetDataPtr(i, j);  break;
					case DB_TYPE_FLOAT: cout <<  *(float*)result_block_ptr->GetDataPtr(i, j);  break;
					default: cout <<  (char*)result_block_ptr->GetDataPtr(i, j);  break;
				}
			}
		}
		uint32_t next = result_block_ptr->NextBlockIndex();
		if(next == 0){ 
			bufferManager->ReleaseBlock((Block* &)result_block_ptr);
			break;
		}
		bufferManager->ReleaseBlock((Block* &)result_block_ptr);
		result_block_ptr =  dynamic_cast<RecordBlock*>(bufferManager->GetBlock(next));
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