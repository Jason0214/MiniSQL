#include "Catalog.h"
#include "../EXCEPTION.h"
#include "../BufferManager/BufferManager.h"
#include "../BufferManager/BlockPtr.h"
#include "../IndexManager/IndexManager.h"

#define SINGLE_DATABSE

using namespace std;

static BufferManager & buffer_manager = BufferManager::Instance();
static IndexManager & index_manager = IndexManager::Instance();

Catalog::Catalog(){
	SchemaBlock* schema_ptr = dynamic_cast<SchemaBlock*>(buffer_manager.GetBlock(0));
	if(schema_ptr->DBMetaAddr() == 0){
		Block* db_block = buffer_manager.CreateBlock(DB_DATABASE_BLOCK);
		db_block->BlockType() = DB_DATABASE_BLOCK;
		db_block->is_dirty = true;
		schema_ptr->DBMetaAddr() = db_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		schema_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(db_block);
	}
	this->database_block_addr = schema_ptr->DBMetaAddr();
	
	if(schema_ptr->UserMetaAddr() == 0){
		Block* user_block = buffer_manager.CreateBlock(DB_USER_BLOCK);
		user_block->BlockType() = DB_USER_BLOCK;
		user_block->is_dirty = true;
		schema_ptr->UserMetaAddr() = user_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		schema_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(user_block);
	}
	this->user_block_addr = schema_ptr->UserMetaAddr();
	buffer_manager.ReleaseBlock(schema_ptr);

	this->table_data_addr = 0;
	this->table_index_addr = 0;
	this->database_selected = false;
#ifdef SINGLE_DATABSE
	try{
		this->UseDatabase("DEFAULT");
	}
	catch(const DatabaseNotFound &){
		this->CreateDatabase("DEFAULT");
		this->UseDatabase("DEFAULT");
	}
#endif
}

void Catalog::CreateDatabase(const string & db_name){
	RecordBlock* database_block_ptr = dynamic_cast<RecordBlock*> (buffer_manager.GetBlock(this->database_block_addr));

	DBenum type_list[5];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	type_list[3] = DB_TYPE_INT;
	type_list[4] = DB_TYPE_INT;
	database_block_ptr->Format(type_list, 5, 0);
	
	while(!database_block_ptr->CheckEmptySpace()){
		uint32_t next_block_index = database_block_ptr->BlockIndex();		
		RecordBlock* next_block_ptr = NULL;
		if(next_block_index == 0){
			next_block_ptr = dynamic_cast <RecordBlock*> (buffer_manager.CreateBlock(DB_RECORD_BLOCK));
			database_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
			database_block_ptr->is_dirty = true;
		}
		buffer_manager.ReleaseBlock(database_block_ptr);
		database_block_ptr = next_block_ptr;
		database_block_ptr->Format(type_list, 5, 0);
	}
	const void* data_list[5];
	BlockPtr<Block> table_data_ptr(buffer_manager.CreateBlock(DB_TABLE_BLOCK));
	BlockPtr<Block> table_index_ptr(buffer_manager.CreateBlock(DB_BPNODE_BLOCK));
	index_manager.initRootBlock(DB_BPTREE_INDEX, table_index_ptr->BlockIndex(), (DBenum)(DB_TYPE_CHAR + 31));
	BlockPtr<Block> index_data_ptr(buffer_manager.CreateBlock(DB_RECORD_BLOCK));
	BlockPtr<Block> index_index_ptr(buffer_manager.CreateBlock(DB_BPNODE_BLOCK));
	index_manager.initRootBlock(DB_BPTREE_INDEX, index_index_ptr->BlockIndex(), (DBenum)(DB_TYPE_CHAR + 32));

	data_list[0] = db_name.c_str();
	data_list[1] = &table_data_ptr->BlockIndex();
	data_list[2] = &table_index_ptr->BlockIndex();
	data_list[3] = &index_data_ptr->BlockIndex();
	data_list[4] = &index_index_ptr->BlockIndex();

	database_block_ptr->InsertTuple(data_list);
	database_block_ptr->is_dirty = true;
	index_index_ptr->is_dirty = true;
	table_index_ptr->is_dirty = true;

	buffer_manager.ReleaseBlock(database_block_ptr);
}

RecordBlock* Catalog::FindDatabaseBlock(const string & db_name){
	DBenum type_list[5];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	type_list[3] = DB_TYPE_INT;
	type_list[4] = DB_TYPE_INT;
	
	uint32_t next_block_addr = this->database_block_addr;
	while(next_block_addr != 0){
		RecordBlock* db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
		db_block_ptr->Format(type_list, 5, 0);
		int i = db_block_ptr->FindTupleIndex(db_name.c_str());
		if(i >= 0 && strcmp((char*)db_block_ptr->GetDataPtr(i, 0), db_name.c_str()) == 0){
			return db_block_ptr;
		}
		next_block_addr = db_block_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock(db_block_ptr);
	}
	throw DatabaseNotFound(db_name);
}

void Catalog::UseDatabase(const string & db_name){
	RecordBlock* block_ptr = this->FindDatabaseBlock(db_name);
	this->current_database_name = db_name;
	uint32_t* database_info = (uint32_t*)(block_ptr->GetDataPtr(block_ptr->FindTupleIndex(db_name.c_str()), 1));
	this->table_data_addr = database_info[0];
	this->table_index_addr = database_info[1];
	this->index_data_addr = database_info[2];
	this->index_index_addr = database_info[3];
	this->database_selected = true;
	buffer_manager.ReleaseBlock(block_ptr);
}

void Catalog::UpdateDatabaseInfo(const string & db_name, unsigned int info_type,uint32_t new_addr){
	RecordBlock* block_ptr = this->FindDatabaseBlock(db_name);
	uint32_t* database_info = (uint32_t*)(block_ptr->GetDataPtr(block_ptr->FindTupleIndex(db_name.c_str()), 1));
	database_info[info_type] = new_addr;
	block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock(block_ptr);
}

void Catalog::CreateTable(const string & table_name, string* attr_name_list, DBenum* attr_type_list, int attr_num, int & key_index){
	if(!this->database_selected) throw DatabaseNotSelected("data base not selected!");

/* find the block where `table_name` should insert in */
	DBenum char_31 = (DBenum)(DB_TYPE_CHAR + 31);
	uint32_t index_tree_root = this->table_index_addr;
	// smart pointer
	AutoPtr<SearchResult> result_ptr(
							index_manager.searchEntry(DB_BPTREE_INDEX, index_tree_root, char_31, table_name.c_str())
						);

	uint32_t table_block_addr;
	// check whether SearchResult is NULL
	if(result_ptr.raw_ptr){
		if(compare(table_name.c_str(), result_ptr->node->getKey(result_ptr->index), char_31) == 0){
			throw DuplicatedTableName(table_name);
		}
		else{
			if (result_ptr->index != 0) {
				result_ptr->index--;
			}
			table_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
		}
	}
	else{
	// first record to insert
		table_block_addr = this->table_data_addr;
	}

/* do real insertion */
	// create smart pointer of TableBlock*
	BlockPtr<TableBlock> table_block_ptr(
						dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr))
					);
	Block* new_table_block = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	uint32_t table_addr = new_table_block->BlockIndex();
	uint32_t index_addr = 0;
	buffer_manager.ReleaseBlock(new_table_block);

	// create the first index block for primary key
	int real_key = key_index < 0 ? 0 : key_index;
	Block* new_index_block = buffer_manager.CreateBlock(DB_BPNODE_BLOCK);
	buffer_manager.ReleaseBlock(new_index_block);
	index_addr = new_index_block->BlockIndex();
	index_manager.initRootBlock(DB_BPTREE_INDEX, index_addr, attr_type_list[real_key]);

	try {
		table_block_ptr->InsertTable(table_name.c_str(), table_addr, index_addr, (uint8_t)attr_num, (uint8_t)key_index);
	}
	catch (const DuplicatedTableName & e) {
		throw e;
	}
	for(int i = 0; i < attr_num; i++){
		table_block_ptr->InsertAttr(attr_name_list[i].c_str(),attr_type_list[i]);
	}
	table_block_ptr->is_dirty = true;

/* update index entry*/
	// calculate the size of the record
	short size = TableBlock::TABLE_RECORD_SIZE + TableBlock::ATTR_RECORD_SIZE * attr_num;
	if (!result_ptr.raw_ptr) {
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, char_31, table_name.c_str(), this->table_data_addr);
	}
	else if (strcmp((char*)table_block_ptr->GetTableInfoPtr(0), table_name.c_str()) == 0) {
		index_tree_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_tree_root, char_31, result_ptr.raw_ptr);
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, char_31, table_name.c_str(), table_block_addr);
	}
	if (size > table_block_ptr->EmptySize()) {
		TableBlock* new_block_ptr = this->SplitTableBlock(table_block_ptr.raw_ptr);
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, char_31,
									new_block_ptr->GetTableInfoPtr(0), new_block_ptr->BlockIndex());
		buffer_manager.ReleaseBlock(new_block_ptr);
	}
	if (index_tree_root != this->table_index_addr) {
		this->UpdateDatabaseTableIndex(this->current_database_name, index_tree_root);
		this->table_index_addr = index_tree_root;
	}
}

void Catalog::DeleteTable(const string & table_name){
	AutoPtr<TableMeta> table_meta(this->GetTableMeta(table_name));
	if(table_meta->key_index >= 0){
		DBenum key_type = table_meta->attr_type_list[table_meta->key_index];
		uint32_t index_tree_root = table_meta->primary_index_addr;
		index_manager.removeIndex(DB_BPTREE_INDEX, index_tree_root, key_type);
	}
	RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(table_meta->table_addr));
	while (data_block_ptr->NextBlockIndex()!= 0) {
		Block* next_block_ptr = buffer_manager.GetBlock(data_block_ptr->NextBlockIndex());
		data_block_ptr->NextBlockIndex() = next_block_ptr->NextBlockIndex();
		buffer_manager.DeleteBlock(next_block_ptr);
	}
	data_block_ptr->RecordNum() = 0;
	data_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock(data_block_ptr);
}

void Catalog::DropTable(const string & table_name){
	if(!this->database_selected) throw DatabaseNotSelected("database not selected!");
/* delete data */
	this->DeleteTable(table_name);
/* search */
	DBenum char_31 = (DBenum)(DB_TYPE_CHAR + 31);
	uint32_t index_tree_root = this->table_index_addr;
	AutoPtr<SearchResult> result_ptr(
							index_manager.searchEntry(DB_BPTREE_INDEX, index_tree_root, char_31, table_name.c_str())
						);

	uint32_t table_block_addr;
	if(result_ptr.raw_ptr){
		if(compare(table_name.c_str(), result_ptr->node->getKey(result_ptr->index), char_31) == 0){
			table_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
		}
		else{
			if(result_ptr->index == 0){
				throw TableNotFound("table not found :" + table_name);
			}
			else{
				table_block_addr = result_ptr->node->addrs()[result_ptr->index];
				result_ptr->index--;
			}
		}		
	}
	else{
		throw TableNotFound(table_name);
	}

/* real deletion */
	BlockPtr<TableBlock> table_block_ptr(
							dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr))
						);
	// find record
	int i = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(i < 0 || strcmp((char*)table_block_ptr->GetTableInfoPtr(i), table_name.c_str()) != 0){
		throw TableNotFound(table_name);
	}
	// delete index and data

	uint32_t primary_index_addr = *(uint32_t*)(table_block_ptr->GetTableInfoPtr(i) + 40);
	Block* primary_index_ptr = buffer_manager.GetBlock(primary_index_addr);
	buffer_manager.DeleteBlock(primary_index_ptr);

	Block* table_data_ptr = buffer_manager.GetBlock(*(uint32_t*)(table_block_ptr->GetTableInfoPtr(i) + 36));
	buffer_manager.DeleteBlock(table_data_ptr);
	
	// delete meta
	table_block_ptr->DropTable(table_name.c_str());

/* update index */
	if(i == 0 && table_block_ptr->RecordNum() > 0){
		index_tree_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_tree_root, char_31, result_ptr.raw_ptr);
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, char_31,
							table_block_ptr->GetTableInfoPtr(0), table_block_ptr->BlockIndex());
	}
	else if(table_block_ptr->RecordNum() == 0){
		index_tree_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_tree_root, char_31, result_ptr.raw_ptr);
		// the block would be empty remove it from link list
		if(table_block_ptr->PreBlockIndex() == 0 && table_block_ptr->NextBlockIndex() == 0){}
		else if(table_block_ptr->PreBlockIndex() == 0){
			this->UpdateDatabaseTableData(this->current_database_name, table_block_ptr->NextBlockIndex());
			this->table_data_addr = table_block_ptr->NextBlockIndex();
			Block* next_block_ptr = buffer_manager.GetBlock(table_block_ptr->NextBlockIndex());
			next_block_ptr->PreBlockIndex() = 0;
			next_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(next_block_ptr);
			buffer_manager.DeleteBlock(table_block_ptr.raw_ptr);
		}
		else if(table_block_ptr->NextBlockIndex() == 0){
			Block* pre_block_ptr = buffer_manager.GetBlock(table_block_ptr->PreBlockIndex());
			pre_block_ptr->NextBlockIndex() = 0;
			pre_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock(table_block_ptr.raw_ptr);
		}
		else{
			Block* pre_block_ptr = buffer_manager.GetBlock(table_block_ptr->PreBlockIndex());
			Block* next_block_ptr = buffer_manager.GetBlock(table_block_ptr->NextBlockIndex());
			pre_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
			next_block_ptr->PreBlockIndex() = pre_block_ptr->BlockIndex();
			pre_block_ptr->is_dirty = true;
			next_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(next_block_ptr);
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock(table_block_ptr.raw_ptr);
		}
	}
	if(index_tree_root != this->table_index_addr){
		this->UpdateDatabaseTableIndex(this->current_database_name, index_tree_root);
		this->table_index_addr = index_tree_root;
	}
}

// split the block pointer by `table_block_ptr`
TableBlock* Catalog::SplitTableBlock(TableBlock* table_block_ptr){
	// create a new table block and move half of data in `table_block_ptr` to the new block	
	TableBlock* new_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.CreateBlock(DB_TABLE_BLOCK));
	// maintain the list
	if(table_block_ptr->NextBlockIndex() != 0){
		Block* next_block_ptr = buffer_manager.GetBlock(table_block_ptr->NextBlockIndex());
		next_block_ptr->PreBlockIndex() = new_block_ptr->BlockIndex();
		next_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(next_block_ptr);
	}
	new_block_ptr->NextBlockIndex() = table_block_ptr->NextBlockIndex();
	table_block_ptr->NextBlockIndex() = new_block_ptr->BlockIndex();
	new_block_ptr->PreBlockIndex() = table_block_ptr->BlockIndex();

	// move attributes
	char _table_name[32];
	char _attr_name[32];
	DBenum _attr_type;
	uint32_t _table_addr;
	uint32_t _index_addr;
	uint16_t _attr_addr;
	uint8_t _attr_num;
	uint8_t _key_index;
	// split the old, move the last half of records to the new table block
	for(int i = 0; i < table_block_ptr->RecordNum()/2; i++){
		memcpy(_table_name, table_block_ptr->GetTableInfoPtr(table_block_ptr->RecordNum()-1), 32);
		table_block_ptr->GetTableMeta(_table_name, _table_addr, _index_addr, _attr_num, _attr_addr, _key_index);
		new_block_ptr->InsertTable(_table_name, _table_addr, _index_addr, _attr_num, _key_index);
		for(unsigned int j = 0; j < _attr_num; j++){
			table_block_ptr->GetAttrMeta(_attr_name, _attr_type, _attr_addr);//change order
			_attr_addr -= TableBlock::ATTR_RECORD_SIZE;
			new_block_ptr->InsertAttr(_attr_name,_attr_type);
		}
		table_block_ptr->DropTable(_table_name);
	}
	table_block_ptr->is_dirty = true;
	new_block_ptr->is_dirty = true;
	return new_block_ptr;
}

uint32_t Catalog::FindTableBlock(const std::string & table_name){
	if(!this->database_selected) throw DatabaseNotSelected("database not selected");
/* find the record of `table_name` */
	DBenum char_31 = (DBenum)(DB_TYPE_CHAR + 31);
	// assignment for check whether root has been modified after implement to index_manager
	uint32_t index_tree_root = this->table_index_addr;
	// smart pointer
	AutoPtr<SearchResult> result_ptr(
						index_manager.searchEntry(DB_BPTREE_INDEX, index_tree_root, char_31, table_name.c_str())
					);
	if(!result_ptr.raw_ptr){
		throw TableNotFound(table_name);
	}
	uint32_t table_block_addr;
	if(compare(result_ptr->node->getKey(result_ptr->index), table_name.c_str(),char_31) == 0){
		table_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
	}
	else if(result_ptr->index != 0){
		table_block_addr = result_ptr->node->addrs()[result_ptr->index];
	}
	else{
	// table_name less than the smallest key in index
		throw TableNotFound(table_name);
	}
	return table_block_addr;
}


void Catalog::UpdateTablePrimaryIndex(const std::string & table_name, uint32_t new_addr){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	BlockPtr<TableBlock> table_block_ptr(dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr)));

	int row = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(row < 0 || strcmp(table_name.c_str(), (char*)table_block_ptr->GetTableInfoPtr(row)) != 0){
		throw TableNotFound( table_name);
	}
	*(uint32_t*)(table_block_ptr->GetTableInfoPtr(row) + 40) = new_addr;
	table_block_ptr->is_dirty = true;
}

void Catalog::UpdateTableDataAddr(const std::string & table_name, uint32_t new_addr){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	BlockPtr<TableBlock> table_block_ptr(dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr)));

	unsigned short row = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(strcmp(table_name.c_str(), (char*)table_block_ptr->GetTableInfoPtr(row)) != 0){
		throw TableNotFound(table_name);
	}
	*(uint32_t*)(table_block_ptr->GetTableInfoPtr(row) + 36) = new_addr;
	table_block_ptr->is_dirty = true;
}

TableMeta* Catalog::GetTableMeta(const string & table_name){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	BlockPtr<TableBlock> table_block_ptr(dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr)));

/*  load data to TableMeta structure*/
	uint16_t attr_addr = 0;
	uint8_t attr_num, key_index;
	TableMeta*  ret = new TableMeta(table_name);
	try{
		table_block_ptr->GetTableMeta(ret->table_name.c_str(), ret->table_addr, ret->primary_index_addr, attr_num,  attr_addr, key_index);
	}
	catch(const TableNotFound & e){
		delete ret;
		throw e;
	}
	ret->attr_num = (int8_t)attr_num;
	if ((int8_t)key_index >= 0) {
		ret->key_index = (int8_t)key_index;
		ret->is_primary_key = true;
	}
	else {
		ret->key_index = 0;
		ret->is_primary_key = false;
	}
	ret->attr_name_list = new string[attr_num];
	ret->attr_type_list = new DBenum[attr_num];
	char buf[32];
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->GetAttrMeta(buf, ret->attr_type_list[i], attr_addr);
		attr_addr -= TableBlock::ATTR_RECORD_SIZE;
		ret->attr_name_list[i] = string(buf);
	};
	return ret;
}

void Catalog::CreateIndex(const string & index_name, const string & table_name, const string & attr_name){
	if(!this->database_selected) throw DatabaseNotSelected("database not selected");
	// check whether same index_name has appeared before,
	// if a IndexNotFound error raised, it's a valid index_name.
	BlockPtr<Block> block_get_by_index_name(NULL);
	try{
		block_get_by_index_name.raw_ptr = this->FindIndexByName(index_name);
	}
	catch(const IndexNotFound &){}
	if(block_get_by_index_name.raw_ptr){
		throw DuplicatedIndexName(index_name);
	}

	AutoPtr<TableMeta> table_meta = this->GetTableMeta(table_name);
	int secondary_key_index = -1;
	DBenum type;
	for (int i = 0; i < table_meta->attr_num; i++) {
		if (table_meta->attr_name_list[i] == attr_name) {
			secondary_key_index = i;
			type = table_meta->attr_type_list[i];
		}
	}
	if (secondary_key_index < 0) {
		throw AttributeNotFound(attr_name);
	}
/* do finding */
	// mix table name and index key, the combination is distinct
	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);

	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	uint32_t index_tree_root = this->index_index_addr;
	AutoPtr<SearchResult> result_ptr = index_manager.searchEntry(DB_BPTREE_INDEX ,index_tree_root, type_list[0], table_name_mix_key.c_str());

	uint32_t index_block_addr;
	if(result_ptr.raw_ptr){
		if(compare(table_name_mix_key.c_str(), result_ptr->node->getKey(result_ptr->index), type_list[0]) == 0){
			throw DuplicatedIndex(table_name.c_str(), secondary_key_index);			
		}
		else{
			if(result_ptr->index == 0){
				index_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
			}
			else{
				result_ptr->index--;
				index_block_addr = result_ptr->node->addrs()[result_ptr->index];
			}
		}
	}
	else{
		index_block_addr = this->index_data_addr;
	}
				
/* do real insertion record */

	RecordBlock* index_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(index_block_addr));
	index_block_ptr->Format(type_list, 3, 0);

	Block* record_index_root_block = buffer_manager.CreateBlock(DB_BPNODE_BLOCK);
	uint32_t record_index_root = record_index_root_block->BlockIndex();
	buffer_manager.ReleaseBlock(record_index_root_block);
	index_manager.initRootBlock(DB_BPTREE_INDEX , record_index_root, type);

	uint32_t next_block_addr = table_meta->table_addr;
	while (next_block_addr != 0) {
		RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));	
		data_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
		record_index_root = index_manager.insertEntry(DB_BPTREE_INDEX, record_index_root, table_meta->attr_type_list[table_meta->key_index],
											data_block_ptr->GetDataPtr(0, table_meta->key_index), data_block_ptr->BlockIndex());
		next_block_addr = data_block_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock(data_block_ptr);
	}

	const void* data_list[3];
	data_list[0] = table_name_mix_key.c_str();
	data_list[1] = index_name.c_str();
	data_list[2] = &record_index_root;

	index_block_ptr->InsertTuple(data_list);

/* update index */
	if(!result_ptr.raw_ptr){
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX ,index_tree_root, type_list[0],
						data_list[0], index_block_ptr->BlockIndex());
	}
	else if(strcmp(table_name_mix_key.c_str(), (char*)index_block_ptr->GetDataPtr(0, 0)) == 0){
		index_tree_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_tree_root, type_list[0], result_ptr.raw_ptr);
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, type_list[0],
						data_list[0], index_block_ptr->BlockIndex());
	}
	// split if needed
	if(!index_block_ptr->CheckEmptySpace()){
		RecordBlock* new_block_ptr = this->SplitRecordBlock(index_block_ptr, type_list, 3, 0);
		index_tree_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_tree_root, type_list[0],
						new_block_ptr->GetDataPtr(0,0), new_block_ptr->BlockIndex());
		buffer_manager.ReleaseBlock(new_block_ptr);
	}
	if(index_tree_root != this->index_index_addr){
		this->UpdateDatabaseIndexIndex(this->current_database_name, index_tree_root);
		this->index_index_addr = index_tree_root;
	}
	index_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock(index_block_ptr);
}

RecordBlock* Catalog::FindIndexByName(const string & index_name){
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	uint32_t next_block_addr = this->index_data_addr;
	while(next_block_addr != 0){
		RecordBlock* index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
		index_data_ptr->Format(type_list, 3, 0);
		for(unsigned int i = 0; i < index_data_ptr->RecordNum(); i++){
			if(strcmp((char*)index_data_ptr->GetDataPtr(i, 1), index_name.c_str()) == 0){
				return index_data_ptr;
			}
		}
		next_block_addr = index_data_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock(index_data_ptr);
	}
	throw IndexNotFound();
}

void Catalog::DropIndex(const string & index_name){
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;


	// traversal all the index_name record, 
	// find the one to be deleted;
	// or throw IndexNotFound.
	bool flag = false;
	uint32_t index_root_block;
	string table_name_mix_key;
	uint32_t next_block_addr = this->index_data_addr;
	RecordBlock* index_data_ptr;
	while(next_block_addr != 0){
		index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->index_data_addr));
		index_data_ptr->Format(type_list, 3, 0);
		int record_num = index_data_ptr->RecordNum();
		for(int i = record_num - 1; i >=0; i--){
			if(strcmp((char*)index_data_ptr->GetDataPtr(i, 1), index_name.c_str()) == 0){
				index_root_block = *(uint32_t*)index_data_ptr->GetDataPtr(i, 2);
				table_name_mix_key = string((char*)(index_data_ptr->GetDataPtr(i, 1)));
				if(i == 0 && index_data_ptr->RecordNum() > 1){
					//update primary index
					uint32_t index_root = this->index_index_addr;
					SearchResult* index_data_entry = index_manager.searchEntry(DB_BPTREE_INDEX ,index_root, type_list[0], index_data_ptr->GetDataPtr(0, 0));
					index_root = index_manager.removeEntry(DB_BPTREE_INDEX ,index_root, type_list[0], index_data_entry);
					index_root = index_manager.insertEntry(DB_BPTREE_INDEX ,index_root, type_list[0], 
											index_data_ptr->GetDataPtr(1, 0), index_data_ptr->BlockIndex());
					delete index_data_entry;
				}
				index_data_ptr->RemoveTuple(i);
				flag = true;
				break;
			}
		}
		if(flag) break;
		next_block_addr = index_data_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock(index_data_ptr);
	}
	if(next_block_addr == 0){
		throw IndexNotFound();
	}

	// drop the whole BP tree;
	TableMeta* table_meta = this->GetTableMeta(table_name_mix_key.substr(0, table_name_mix_key.size() - 1));
	index_manager.removeIndex(DB_BPTREE_INDEX, index_root_block, table_meta->attr_type_list[table_meta->key_index]);
	delete table_meta;

	if(index_data_ptr->RecordNum() == 0){
		if(index_data_ptr->PreBlockIndex() == 0 && index_data_ptr->NextBlockIndex() == 0){
		}
		else if(index_data_ptr->PreBlockIndex() == 0){
			this->UpdateDatabaseIndexData(this->current_database_name, index_data_ptr->NextBlockIndex());
			this->index_data_addr = index_data_ptr->NextBlockIndex();
			Block* next_block_ptr = buffer_manager.GetBlock(index_data_ptr->NextBlockIndex());
			next_block_ptr->PreBlockIndex() = 0;
			next_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(next_block_ptr);
			buffer_manager.DeleteBlock(index_data_ptr);
		}
		else if(index_data_ptr->NextBlockIndex() == 0){
			Block* pre_block_ptr = buffer_manager.GetBlock(index_data_ptr->PreBlockIndex());
			pre_block_ptr->NextBlockIndex() = 0;
			pre_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock(index_data_ptr);
		}
		else{
			Block* pre_block_ptr = buffer_manager.GetBlock(index_data_ptr->PreBlockIndex());
			Block* next_block_ptr = buffer_manager.GetBlock(index_data_ptr->NextBlockIndex());
			pre_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
			next_block_ptr->PreBlockIndex() = pre_block_ptr->BlockIndex();
			pre_block_ptr->is_dirty = true;
			next_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(next_block_ptr);
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock(index_data_ptr);
		}
	}
	if(index_data_ptr){
		index_data_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(index_data_ptr);
	}
}

RecordBlock* Catalog::SplitRecordBlock(RecordBlock* origin_block_ptr, DBenum* types, int8_t num, int8_t key){
	// create a new table block and move half of data in `record_block_ptr` to the new block	
	RecordBlock* new_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.CreateBlock(DB_RECORD_BLOCK));
	// maintain the list
	if(origin_block_ptr->NextBlockIndex() != 0){
		Block* next_block_ptr = buffer_manager.GetBlock(origin_block_ptr->NextBlockIndex());
		next_block_ptr->PreBlockIndex() = new_block_ptr->BlockIndex();
		next_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock(next_block_ptr);
	}
	new_block_ptr->PreBlockIndex() = origin_block_ptr->BlockIndex();
	new_block_ptr->NextBlockIndex() = origin_block_ptr->NextBlockIndex();
	origin_block_ptr->NextBlockIndex() = new_block_ptr->BlockIndex();

	new_block_ptr->Format(types, num, key);

	unsigned int half_of_records = origin_block_ptr->RecordNum()/2;
	memcpy(new_block_ptr->GetDataPtr(0, 0), origin_block_ptr->GetDataPtr(half_of_records, 0), 
					(size_t)(new_block_ptr->tuple_size*(origin_block_ptr->RecordNum() - half_of_records)));
	new_block_ptr->RecordNum() = origin_block_ptr->RecordNum() - half_of_records;
	origin_block_ptr->RecordNum() = half_of_records;
	new_block_ptr->is_dirty = true;
	origin_block_ptr->is_dirty = true;
	return new_block_ptr;
}

uint32_t Catalog::FindIndexBlock(const std::string & table_name_mix_key){
	if (!this->database_selected) throw DatabaseNotSelected("database not selected");
/* find through index */
	DBenum char_32 = (DBenum)(DB_TYPE_CHAR + 32);
	uint32_t index_tree_root = this->index_index_addr;
	AutoPtr<SearchResult> result_ptr(index_manager.searchEntry(DB_BPTREE_INDEX, index_tree_root, char_32, table_name_mix_key.c_str()));
	if(!result_ptr.raw_ptr){
		throw IndexNotFound();
	}
	uint32_t record_block_addr;
	if(compare(result_ptr->node->getKey(result_ptr->index) ,table_name_mix_key.c_str(), char_32) == 0){
		record_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
	}
	else if(result_ptr->index != 0){
		record_block_addr = result_ptr->node->addrs()[result_ptr->index];
	}
	else{
		throw IndexNotFound();		
	}
	return record_block_addr;
}

void Catalog::UpdateTableSecondaryIndex(const std::string & table_name, int8_t key_index, uint32_t new_addr){
	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&key_index);
	uint32_t record_block_addr;
	try {
		 record_block_addr = this->FindIndexBlock(table_name_mix_key);
	}
	catch(IndexNotFound & e){
		throw e;
	}
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(record_block_addr));
	record_block_ptr->Format(type_list, 3, 0);
	int i = record_block_ptr->FindTupleIndex(table_name_mix_key.c_str());
	if(i < 0 || strcmp(table_name_mix_key.c_str(), (char*)record_block_ptr->GetDataPtr(i, 0)) != 0){
		buffer_manager.ReleaseBlock(record_block_ptr);
		throw IndexNotFound();
	}

	*(uint32_t*)record_block_ptr->GetDataPtr(i, 2) = new_addr;
	record_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock(record_block_ptr);
}


uint32_t Catalog::GetIndex(const string & table_name, int8_t secondary_key_index){
	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);
	uint32_t record_block_addr;
	try {
		 record_block_addr = this->FindIndexBlock(table_name_mix_key);
	}
	catch(IndexNotFound & e){
		throw e;
	}

	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;
	uint32_t ret;
	RecordBlock* record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(record_block_addr));
	record_block_ptr->Format(type_list, 3, 0);

	int i = record_block_ptr->FindTupleIndex(table_name_mix_key.c_str());
	if(i < 0 || strcmp(table_name_mix_key.c_str(), (char*)record_block_ptr->GetDataPtr(i, 0)) != 0){
		buffer_manager.ReleaseBlock(record_block_ptr);
		throw IndexNotFound();
	}
	ret = *(uint32_t*)record_block_ptr->GetDataPtr(i, 2);
	buffer_manager.ReleaseBlock(record_block_ptr);
	return ret;
}