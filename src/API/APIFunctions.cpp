#include "APIFunctions.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "APIStructures.h"

#include "../BufferManager/Block.h"
#include "../BufferManager/BlockPtr.h"
#include "../BufferManager/BufferManager.h"
#include "../CatalogManager/Catalog.h"
#include "../IndexManager/IndexManager.h"
#include "../RecordManager/RecordManager.h"

//#define __DEBUG__

using namespace std;
static BufferManager & buffer_manager = BufferManager::Instance();
static IndexManager & index_manager = IndexManager::Instance();
static Catalog & catalog = Catalog::Instance();
static RecordManager & record_manager = RecordManager::Instance();

// call Flush() after cout.
// Do not call cin, call GetString() / GetInt() / GetFloat() if necessary

void BeginQuery()
{

}

void EndQuery()
{
	record_manager.clear();
}

void ExeExit(){
	buffer_manager.RemoveAllBlock();
}
//Assumption: the first operand is always attribute
//the second is always a constant
//cmpVec is sorted
void ExeSelect(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const ComparisonVector& cmpVec)
{
	// convert alias to real table name
	string tableName;
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (exception& e) {
		throw TableAliasNotFound(sourceTableName);
	}
	
	// get src table
	Table* src_table;
	try {
		src_table = record_manager.getTable(tableName);
	}
	catch (const exception &) {
		src_table = new Table(tableName);
		record_manager.addTable(src_table);
	}

	// create dst table
	Table* dst_table = new Table(resultTableName, src_table);
	record_manager.addTable(dst_table);
	
	//  pre-check
	bool has_primary_index = false;
	string primaryAttrName = src_table->getAttrName(src_table->getKeyIndex());
	string operation;
	string value;
	for(auto i = cmpVec.begin(); i < cmpVec.end(); i++){
		if(primaryAttrName == i->Comparand1.Content){
			operation = i->Operation;
			value = i->Comparand2.Content;
			has_primary_index = true;
			break;
		}
	}

	TableIterator* begin;
	TableIterator* end;
	if(has_primary_index){
		pair<TableIterator*, TableIterator*> temp = src_table->PrimaryIndexFilter(operation, value);
		begin = temp.first;
		end = temp.second;
	}
	else{
		begin = src_table->begin();
		end = src_table->end();
	}

	// find data type corresponding to cmpVec
	TupleComparisonVector tuple_cmp_vec;
	RecordManager::CmpVec2TupleCmpVec(src_table, cmpVec, tuple_cmp_vec);

	const void** tuple_data_ptr = new const void*[dst_table->getAttrNum()];
	for(TableIterator* iter = begin; !iter->isEqual(end); iter->next()){
		for (int i = 0; i < dst_table->getAttrNum(); i++) {
			tuple_data_ptr[i] = iter->getAttrData(i);
		}
		if(RecordManager::ConditionCheck(tuple_data_ptr, tuple_cmp_vec)){
			dst_table->insertTuple(tuple_data_ptr);
		}
	}
	delete begin;
	delete end;
	delete [] tuple_data_ptr;
#ifdef __DEBUG__
	cout << "select result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}


//attrVec is sorted
void ExeProject(const TableAliasMap& tableAlias, const string& sourceTableName,
	const string& resultTableName, const AttrNameAliasVector& attrVec)
{
	string tableName;
	//check if alias exists
	try {
		tableName = tableAlias.at(sourceTableName);
	}
	catch (const exception& e) {
		throw TableAliasNotFound(sourceTableName);
	}
	Table* src_table;
	try{
		src_table = record_manager.getTable(tableName);
	}
	catch(const exception &){
		src_table = new Table(tableName);
		record_manager.addTable(src_table);
	}

	//get new attr index and alias
	AttributesAliasVector attr_alias;
	for (int i = 0, j = 0; i < src_table->getAttrNum() && j < (int)attrVec.size(); i++) {
		if (src_table->getAttrName(i) == attrVec[j].AttrName) {
			AliasStructure buf;
			buf.AttrName = attrVec[j].AttrAlias;
			buf.OriginIndex = i;
			attr_alias.push_back(buf);
			j++;
		}
	}
	sort(attr_alias.begin(), attr_alias.end(), AttrAliasSort);
	
	Table* dst_table = new Table(resultTableName, src_table, attr_alias);
	record_manager.addTable(dst_table);

	const void** tuple_data_ptr = new const void*[dst_table->getAttrNum()];

	TableIterator* begin = src_table->begin();
	TableIterator* end = src_table->end();
	for(TableIterator* iter = begin; !iter->isEqual(end); iter->next()){
		for(unsigned int i = 0; i < attr_alias.size(); i++){
			tuple_data_ptr[i] = iter->getAttrData(attr_alias[i].OriginIndex);
		}
		dst_table->insertTuple(tuple_data_ptr);
	}
	delete [] tuple_data_ptr;
	delete begin;
	delete end;
#ifdef __DEBUG__
	cout << "project result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

//attr is sorted
void ExeNaturalJoin(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{
	// get real table name
	string tableName1;
	string tableName2;
	try {
		tableName1 = tableAlias.at(sourceTableName1);
	}
	catch (const exception& ) {
		throw TableAliasNotFound(sourceTableName1);
	}
	try {
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (const exception& ) {
		throw TableAliasNotFound(sourceTableName2);
	}
	
	// get table 
	Table* src_table1;
	Table* src_table2;
	try{
		src_table1 = record_manager.getTable(tableName1);
	}
	catch(const exception & ){
		src_table1 = new Table(tableName1);
		record_manager.addTable(src_table1);
	}
	try{
		src_table2 = record_manager.getTable(tableName2);
	} 
	catch(const exception &){
		src_table2 = new Table(tableName2);
		record_manager.addTable(src_table2);
	}

	//get common attrs
	//attr_num is sorted
	vector<pair<int, int> > commonDstAttrIndex;
	AttributesAliasVector attr_alias;
    AliasStructure buf;
    int i,j;
	for (i = 0, j = 0;i < src_table1->getAttrNum() && j < src_table2->getAttrNum();) {
		if (src_table1->getAttrName(i) > src_table2->getAttrName(j)){
			// use inverse of origin index when attribute is in src table 2
			buf.AttrName = src_table2->getAttrName(j);
			buf.OriginIndex = ~j;
			attr_alias.push_back(buf);
			j++;
		}
		else if (src_table1->getAttrName(i) < src_table2->getAttrName(j)){
			buf.AttrName = src_table1->getAttrName(i);
			buf.OriginIndex = i;
			attr_alias.push_back(buf);
			i++;
		}
		else { //==
			if(src_table1->getAttrType(i) != src_table2->getAttrType(j)){
				throw SameAttrNameWithDifferType("");
			}

			buf.AttrName = src_table1->getAttrName(i);
			buf.OriginIndex = i;
			attr_alias.push_back(buf);			
			
			commonDstAttrIndex.emplace_back(make_pair(i, j));
			i++;j++;
		}
	}
    while(i < src_table1->getAttrNum()){
        buf.AttrName = src_table1->getAttrName(i);
        buf.OriginIndex = i;
        attr_alias.push_back(buf);
        i++;
    }
    while(j < src_table2->getAttrNum()){
        buf.AttrName = src_table2->getAttrName(j);
        buf.OriginIndex = ~j;
        attr_alias.push_back(buf);
        j++;
    }


	sort(attr_alias.begin(), attr_alias.end(), AttrAliasSort);
	Table* dst_table = new Table(resultTableName, src_table1, src_table2, attr_alias);
	record_manager.addTable(dst_table);

	if(src_table1->flag() == DB_TEMPORAL_TABLE && src_table2->flag() == DB_TEMPORAL_TABLE){
		RecordManager::naturalJoin(DB_NESTED_LOOP_JOIN, src_table1, src_table2, dst_table, attr_alias, commonDstAttrIndex);
	}
	else if(src_table1->flag() == DB_TEMPORAL_TABLE){
		RecordManager::naturalJoin(DB_SINGLE_BLOCK_NESTED_JOIN, src_table2, src_table1, dst_table, attr_alias, commonDstAttrIndex);
	}
	else if(src_table2->flag() == DB_TEMPORAL_TABLE){
		RecordManager::naturalJoin(DB_SINGLE_BLOCK_NESTED_JOIN, src_table1, src_table2, dst_table, attr_alias, commonDstAttrIndex);
	}
	else{
		RecordManager::naturalJoin(DB_DOUBLE_BLOCK_NESTED_JOIN, src_table1, src_table2, dst_table, attr_alias, commonDstAttrIndex);		
	}

#ifdef __DEBUG__
	cout << "Natural join result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

void ExeCartesian(const TableAliasMap& tableAlias, const string& sourceTableName1,
	const string& sourceTableName2, const string& resultTableName)
{
	//make sure table name is valid
	string tableName1, tableName2;
	try {
		tableName1 = tableAlias.at(sourceTableName1);
	}
	catch (const exception& e) {
		throw TableAliasNotFound(sourceTableName1);
	}
	try {
		tableName2 = tableAlias.at(sourceTableName2);
	}
	catch (const exception& e) {
		throw TableAliasNotFound(sourceTableName2);
	}

	Table* src_table1;
	Table* src_table2;
	try{
		src_table1 = record_manager.getTable(tableName1);
	}
	catch(const exception & ){
		src_table1 = new Table(tableName1);
		record_manager.addTable(src_table1);
	}
	try{
		src_table2 = record_manager.getTable(tableName2);
	}
	catch(const exception &){
		src_table2 = new Table(tableName2);
		record_manager.addTable(src_table2);
	}

    // generate new table attributes name
	IndirectAttrMap indirect_attr_map;
	AttributesAliasVector attr_alias;
    AliasStructure buf;
    int i,j;
	for (i = 0, j = 0; i < src_table1->getAttrNum() && j < src_table2->getAttrNum(); ) {
		if (src_table1->getAttrName(i) > src_table2->getAttrName(j)){
			// use inverse of origin index when attribute is in src table 2
			buf.AttrName = src_table2->getAttrName(j);
			buf.OriginIndex = ~j;
			attr_alias.push_back(buf);
			j++;
		}
		else if (src_table1->getAttrName(i) < src_table2->getAttrName(j)){
			buf.AttrName = src_table1->getAttrName(i);
			buf.OriginIndex = i;
			attr_alias.push_back(buf);
			i++;
		}
		else { //==
			buf.AttrName = RecordManager::Alias4IndirectAttr(src_table1->getTableName(), 
										src_table1->getAttrName(i), indirect_attr_map.size());
			buf.OriginIndex = i;
			attr_alias.push_back(buf);			
			indirect_attr_map.insert(make_pair(buf.AttrName, make_pair(src_table1->getTableName(), src_table1->getAttrName(i))));

			buf.AttrName = RecordManager::Alias4IndirectAttr(src_table2->getTableName(), 
										src_table2->getAttrName(j), indirect_attr_map.size());
			buf.OriginIndex = ~j;
			attr_alias.push_back(buf);
			indirect_attr_map.insert(make_pair(buf.AttrName, make_pair(src_table2->getTableName(), src_table2->getAttrName(j))));
			i++;j++;
		}
	}
    while(i < src_table1->getAttrNum()){
        buf.AttrName = src_table1->getAttrName(i);
        buf.OriginIndex = i;
        attr_alias.push_back(buf);
        i++;
    }
    while(j < src_table2->getAttrNum()){
        buf.AttrName = src_table2->getAttrName(j);
        buf.OriginIndex = ~j;
        attr_alias.push_back(buf);
        j++;
    }

	sort(attr_alias.begin(), attr_alias.end(), AttrAliasSort);
	Table* dst_table = new Table(resultTableName, src_table1, src_table2, attr_alias, indirect_attr_map);
	record_manager.addTable(dst_table);
	
	if(src_table1->flag() == DB_TEMPORAL_TABLE && src_table2->flag() == DB_TEMPORAL_TABLE){
		RecordManager::join(DB_NESTED_LOOP_JOIN, src_table1, src_table2, dst_table, attr_alias);
	}
	else if(src_table1->flag() == DB_TEMPORAL_TABLE){
		RecordManager::join(DB_SINGLE_BLOCK_NESTED_JOIN, src_table2, src_table1, dst_table, attr_alias);
	}
	else if(src_table2->flag() == DB_TEMPORAL_TABLE){
		RecordManager::join(DB_SINGLE_BLOCK_NESTED_JOIN, src_table1, src_table2, dst_table, attr_alias);
	}
	else{
		RecordManager::join(DB_DOUBLE_BLOCK_NESTED_JOIN, src_table1, src_table2, dst_table, attr_alias);
	}

#ifdef __DEBUG__
	cout << "Cartesian product result:";
	ExeOutputTable(tableAlias, resultTableName);
#endif
}

//print out a table
void ExeOutputTable(const TableAliasMap& tableAlias, const string& sourceTableName)
{
	string tableName;
	try{
		tableName = tableAlias.at(sourceTableName);
	}
	catch(const exception &){
		throw TableAliasNotFound(sourceTableName);
	}

	Table* table;
	try{
		table = record_manager.getTable(sourceTableName);
	}
	catch(const exception &){
		table = new Table(tableName);
		record_manager.addTable(table);
	}

	cout << endl;
	//adjust align
	cout << left;
	//print out attr name
	unsigned int borderLen = (unsigned int)table->getAttrNum() - 1;
	for (int i = 0; i < table->getAttrNum(); i++) {
		switch (table->getAttrType(i)) {
		case DB_TYPE_INT: borderLen += INTLEN;  break;
		case DB_TYPE_FLOAT: borderLen += FLOATLEN;  break;
		default:  borderLen += STRLEN;  break;
        }
	}
	string horizontalBorder(borderLen, '-');
	horizontalBorder = " " + horizontalBorder + " ";
	cout << horizontalBorder << endl;
	for (int i = 0; i < table->getAttrNum(); i++) {
		cout << "|";
		switch (table->getAttrType(i)) {
		case DB_TYPE_INT: cout << setw(INTLEN);  break;
		case DB_TYPE_FLOAT: cout << setw(FLOATLEN);  break;
		default: cout << setw(STRLEN);  break;
		}
		cout << table->getFullAttrName(i);
	}
	cout << "|" << endl << horizontalBorder << endl;
	
	//print out data
	TableIterator* begin = table->begin();
	TableIterator* end = table->end();
	for(TableIterator* iter = begin; !iter->isEqual(end); iter->next()){
		for (int j = 0; j < table->getAttrNum(); j++) {
			cout << "|";
			printByType(iter->getAttrData(j), table->getAttrType(j));
		}
		cout << "|";
		cout << endl;
	}
	cout << horizontalBorder << endl;
	delete begin;
	delete end;
}

void ExeInsert(const string& tableName, InsertValueVector& values) {
	AutoPtr<Table> table(new Table(tableName));

	if ((unsigned int)(table->getAttrNum()) != values.size()) {
		throw AttrNumberUnmatch();
	}

	Tuple tuple(table->getAttrNum(), table->getAttrTypeList());
    // FIXME: add exception
	for(int i = 0; i < table->getAttrNum(); i++){
		DBenum type = table->getAttrType(i);
		string2Bytes(values[i], type, tuple[i]);
	}
	table->insertTuple(tuple.entry_ptr());
	cout << "1 Row Affected" << endl;
}

static void DeleteTableBlock(Table* table_ptr, RecordBlock* data_block_ptr){
	// if only one record remained in the block
	if (data_block_ptr->PreBlockIndex() == 0 && data_block_ptr->NextBlockIndex() == 0) {}
	else if (data_block_ptr->PreBlockIndex() == 0) {
		catalog.UpdateTableDataAddr(table_ptr->getTableName(), data_block_ptr->NextBlockIndex());
		table_ptr->updateDataBlockAddr(data_block_ptr->NextBlockIndex());
		Block* next_block_ptr = buffer_manager.GetBlock(data_block_ptr->NextBlockIndex());
		next_block_ptr->PreBlockIndex() = 0;
		next_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(next_block_ptr);
		buffer_manager.DeleteBlock(data_block_ptr);
	}
	else if (data_block_ptr->NextBlockIndex() == 0) {
		Block* pre_block_ptr = buffer_manager.GetBlock(data_block_ptr->PreBlockIndex());
		pre_block_ptr->NextBlockIndex() = 0;
		pre_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(pre_block_ptr);
		buffer_manager.DeleteBlock(data_block_ptr);
	}
	else {
		Block* pre_block_ptr = buffer_manager.GetBlock(data_block_ptr->PreBlockIndex());
		Block* next_block_ptr = buffer_manager.GetBlock(data_block_ptr->NextBlockIndex());
		pre_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
		next_block_ptr->PreBlockIndex() = pre_block_ptr->BlockIndex();
		pre_block_ptr->is_dirty = true;
		next_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(next_block_ptr);
		buffer_manager.ReleaseBlock(pre_block_ptr);
		buffer_manager.DeleteBlock(data_block_ptr);
	}
}

void ExeUpdate(const string& tableName, const string& attrName,
	const string& value, const ComparisonVector& cmpVec)
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

	// match cmpVec type
	TupleComparisonVector tuple_cmp_vec;
	RecordManager::CmpVec2TupleCmpVec(table.raw_ptr, cmpVec, tuple_cmp_vec);

	const void** tuple_data_ptr = new const void*[table->getAttrNum()];

	DBenum attr_type = table->getAttrType(attr_index);
	uint32_t index_root = table->getIndexRoot();
	if (attr_index != table->getKeyIndex()) {
		// no need to update primary index
		uint32_t next_block_addr = begin_addr;
		while (next_block_addr != end_addr) {
			RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
			data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
			int record_num = data_block_ptr->RecordNum();
			for (int i = 0; i < record_num; i++) {
				for(int j = 0; j < table->getAttrNum(); j++){
					tuple_data_ptr[j] = data_block_ptr->GetDataPtr(i, j);
				}
				if (RecordManager::ConditionCheck(tuple_data_ptr, tuple_cmp_vec)) {
					data_block_ptr->SetTupleValue(i, attr_index, raw_value);
					updated_tuple_count++;
				}
			}
			next_block_addr = data_block_ptr->NextBlockIndex();
			buffer_manager.ReleaseBlock(data_block_ptr);
		}
	}
	else {
		//pre-check whether the new attribute value would cause duplicated primary key
		if (table->isPrimaryKey()) {
			uint32_t target_block_addr;
			AutoPtr<SearchResult> result_ptr(index_manager.searchEntry(DB_BPTREE_INDEX, index_root, key_type, raw_value));
			if (compare(result_ptr->node->getKey(result_ptr->index), raw_value, key_type) == 0) {
				target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
			}
			else {
				if (result_ptr->index != 0) {
					result_ptr->index--;
				}
				target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
			}
			BlockPtr<RecordBlock> target_block_ptr(dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(target_block_addr)));
			target_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
			int target_index = target_block_ptr->FindTupleIndex(raw_value);
			if (target_index >= 0 && target_index < target_block_ptr->RecordNum() &&
				compare(raw_value, target_block_ptr->GetDataPtr(target_index, table->getKeyIndex()), key_type) == 0) {
				throw DuplicatedPrimaryKey();
			}
		}

		/* update begins here */
		Tuple tuple(table->getAttrNum(), table->getAttrTypeList());
		uint32_t next_block_addr = begin_addr;
		while (next_block_addr != end_addr) {
			RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
			data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
			next_block_addr = data_block_ptr->NextBlockIndex();

			// tempararily remove the index
			SearchResult* data_block_entry = index_manager.searchEntry(DB_BPTREE_INDEX, index_root, key_type, data_block_ptr->GetDataPtr(0, attr_index));
			index_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_root, key_type, data_block_entry);
			delete data_block_entry;

			int record_num = data_block_ptr->RecordNum();
			for (int i = record_num - 1; i >= 0; i--) {
				for(int j = 0; j < table->getAttrNum(); j++){
					tuple_data_ptr[j] = data_block_ptr->GetDataPtr(i, j);
				}
				if (RecordManager::ConditionCheck(tuple_data_ptr, tuple_cmp_vec)) {
					data_block_ptr->SetTupleValue(i, attr_index, raw_value);
					if (compare(raw_value, data_block_ptr->GetDataPtr(0, attr_index), attr_type) < 0
					|| (next_block_addr && compare(raw_value, BlockPtr<RecordBlock>(dynamic_cast<RecordBlock*>
					(buffer_manager.GetBlock(next_block_addr)))->GetDataPtr(0, attr_index), attr_type) >= 0)) {
						memcpy(tuple.data_ptr(), data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
						data_block_ptr->RemoveTuple(i);
						table->insertTuple(tuple.entry_ptr());
						index_root = table->getIndexRoot();
					}
					updated_tuple_count++;
				}
			}
			if (data_block_ptr->RecordNum() != 0) {
				// insertion sort
				for (int i = 1; i < data_block_ptr->RecordNum(); i++) {
					memcpy(tuple.data_ptr(), data_block_ptr->GetDataPtr(i, 0), data_block_ptr->tuple_size);
					int j = i - 1;
					for (; j >= 0 && compare(tuple[attr_index], data_block_ptr->GetDataPtr(j, attr_index), attr_type) < 0; j--) {
						memcpy(data_block_ptr->GetDataPtr(j + 1, 0), data_block_ptr->GetDataPtr(j, 0), data_block_ptr->tuple_size);
					}
					memcpy(data_block_ptr->GetDataPtr(j + 1, 0), tuple.data_ptr(), data_block_ptr->tuple_size);
				}
				// add index back
				index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, 
								data_block_ptr->GetDataPtr(0, attr_index), data_block_ptr->BlockIndex());
			}
			else {
				DeleteTableBlock(table.raw_ptr, data_block_ptr);
			}
			buffer_manager.ReleaseBlock(data_block_ptr);
		}
		if (index_root != table->getIndexRoot()) {
			catalog.UpdateTablePrimaryIndex(tableName, index_root);
			table->updateDataBlockAddr(index_root);
		}
	}
	delete [] tuple_data_ptr;
	delete [] raw_value;
	cout << updated_tuple_count << " Row Affected" << endl;
}

//
void ExeDelete(const string& tableName, const ComparisonVector& cmpVec)
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
	DBenum key_type = table->getAttrType(table->getKeyIndex());
	uint8_t* raw_value = new uint8_t[typeLen(key_type)];
	string2Bytes(cmpVec[key_match].Comparand2.Content, key_type, raw_value);
	uint32_t begin_addr = table->getDataBlockAddr(), end_addr = 0;
	if (key_match != -1) {
		table->BlockFilter(cmpVec[key_match].Operation, raw_value, &begin_addr, &end_addr);
	}
	delete[] raw_value;

	// match cmpVec type
	TupleComparisonVector tuple_cmp_vec;
	RecordManager::CmpVec2TupleCmpVec(table.raw_ptr, cmpVec, tuple_cmp_vec);

	const void** tuple_data_ptr = new const void*[table->getAttrNum()];

	// delete in tuple scale
	uint32_t index_root = table->getIndexRoot();
	uint32_t next_block_addr = begin_addr;
	while (next_block_addr != end_addr) {
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
		data_block_ptr->Format(table->getAttrTypeList(), table->getAttrNum(), table->getKeyIndex());
		int record_num = data_block_ptr->RecordNum();
		DBenum key_type = table->getAttrType(table->getKeyIndex());
		for (int i = record_num - 1; i >= 0; i--) {
			for(int j = 0; j < table->getAttrNum(); j++){
				tuple_data_ptr[j] = data_block_ptr->GetDataPtr(i, j);
			}
			if (RecordManager::ConditionCheck(tuple_data_ptr, tuple_cmp_vec)) {
				if (i == 0) {
					SearchResult* result_ptr = index_manager.searchEntry(DB_BPTREE_INDEX, index_root, key_type,
						data_block_ptr->GetDataPtr(0, table->getKeyIndex()));
					index_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_root, key_type, result_ptr);
					if (data_block_ptr->RecordNum() > 1) {
						index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type,
							data_block_ptr->GetDataPtr(1, table->getKeyIndex()), data_block_ptr->BlockIndex());

					}
					delete result_ptr;
				}
				data_block_ptr->RemoveTuple(i);
				data_block_ptr->is_dirty = true;
				deleted_tuple_count += 1;
			}
		}
		next_block_addr = data_block_ptr->NextBlockIndex();
		if (data_block_ptr->RecordNum() == 0) {
			DeleteTableBlock(table.raw_ptr, data_block_ptr);
		}
		buffer_manager.ReleaseBlock((Block* &)data_block_ptr);
	}
	if (index_root != table->getIndexRoot()) {
		catalog.UpdateTablePrimaryIndex(tableName, index_root);
		table->updateDataBlockAddr(index_root);
	}
	cout << deleted_tuple_count << " Rows Affected" << endl;
	delete [] tuple_data_ptr;
}

void ExeDropIndex(const string& indexName)
{
	try {
		catalog.DropIndex(indexName);
	}
	catch (const IndexNotFound &) {
		cout << "Index `" << indexName << "` Not Found" << endl;
		return;
	}
	cout << "Drop Index Named `" << indexName << "` Successfully" << endl;
}

void ExeDropTable(const string& tableName)
{
	try {
		catalog.DropTable(tableName);
	}
	catch (const TableNotFound) {
		cout << "Table `" << tableName << "` Not Found" << endl;
		return;
	}
	cout << "Drop Table `" << tableName << "` Successfully" << endl;
	return;
}

void ExeCreateIndex(const string& tableName, const string& attrName, const string& indexName)

{
	try {
		catalog.CreateIndex(indexName, tableName, attrName);
	}
	catch (const DuplicatedIndex &) {
		cout << "Duplicated Index `" << indexName << "`" << endl;
		return;
	}
	catch (const TableNotFound & ) {
		cout << "Table `" << tableName << "` Not Found" << endl;
		return;
	}
	catch (const AttributeNotFound &) {
		cout << "Attribute `" << attrName << "` Not Found" << endl;
	}
	cout << "Create Index on `" << tableName << "` Successfully" << endl;
}

void ExeCreateTable(const string& tableName, const AttrDefinitionVector& defVec)
{
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

	// sort attr
	for (int i = 1; i < attr_num; i++) {
		string t1 = attr_name_list[i];
		DBenum t2 = attr_type_list[i];
		int j = i - 1;
		for (; j >= 0 && attr_name_list[j] > t1; j--) {
			attr_name_list[j + 1] = attr_name_list[j];
			attr_type_list[j + 1] = attr_type_list[j];
		}
		attr_name_list[j + 1] = t1;
		attr_type_list[j + 1] = t2;
		if (key_index == i) key_index = j + 1;
		else if (key_index > j && key_index < i) key_index++;
	}
	
	// do create
	try {
		catalog.CreateTable(tableName, attr_name_list, attr_type_list, attr_num, key_index);
	}
	catch (const DuplicatedTableName & e) {
		delete [] attr_name_list;
		delete[] attr_type_list;
		throw e;
	}
	cout << "Create Table `" << tableName << "` Successfully" << endl;
	delete[] attr_name_list;
	delete[] attr_type_list;
}