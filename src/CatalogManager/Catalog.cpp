#include "Catalog.h"
#include "../EXCEPTION.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"
#include "../Type/ConstChar.h"

#define SINGLE_DATABSE

using namespace std;

static BufferManager & buffer_manager = BufferManager::Instance();

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
	buffer_manager.ReleaseBlock((Block* &)schema_ptr);

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
		buffer_manager.ReleaseBlock((Block* &)database_block_ptr);
		database_block_ptr = next_block_ptr;
		database_block_ptr->Format(type_list, 5, 0);
	}
	const void* data_list[5];
	Block* table_data_ptr = buffer_manager.CreateBlock(DB_TABLE_BLOCK);
	Block* table_index_ptr = buffer_manager.CreateBlock();
	this->InitBPIndexRoot(table_index_ptr, (DBenum)(DB_TYPE_CHAR + 31));
	Block* index_data_ptr = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	Block* index_index_ptr = buffer_manager.CreateBlock();
	this->InitBPIndexRoot(index_index_ptr, (DBenum)(DB_TYPE_CHAR + 32));

	data_list[0] = db_name.c_str();
	data_list[1] = &table_data_ptr->BlockIndex();
	data_list[2] = &table_index_ptr->BlockIndex();
	data_list[3] = &index_data_ptr->BlockIndex();
	data_list[4] = &index_index_ptr->BlockIndex();

	database_block_ptr->InsertTuple(data_list);
	database_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock(table_data_ptr);
	buffer_manager.ReleaseBlock(table_index_ptr);
	buffer_manager.ReleaseBlock(index_data_ptr);
	buffer_manager.ReleaseBlock(index_index_ptr);
	buffer_manager.ReleaseBlock((Block* &)database_block_ptr);
}

RecordBlock* Catalog::FindDatabaseBlock(const string & db_name){
	DBenum type_list[5];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	type_list[3] = DB_TYPE_INT;
	type_list[4] = DB_TYPE_INT;
	RecordBlock* db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->database_block_addr));
	db_block_ptr->Format(type_list, 5, 0);

	while(true){
		int i = db_block_ptr->FindTupleIndex(db_name.c_str());
		if(i >= 0 && strcmp((char*)db_block_ptr->GetDataPtr(i, 0), db_name.c_str()) == 0){
			break;
		}
		uint32_t next = db_block_ptr->NextBlockIndex();		
		buffer_manager.ReleaseBlock((Block* &)db_block_ptr);
		if(next == 0) throw DatabaseNotFound("database not found!");
		db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
		db_block_ptr->Format(type_list, 5, 0);
	}
	return db_block_ptr;
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
	buffer_manager.ReleaseBlock((Block* &)block_ptr);
}

void Catalog::UpdateDatabaseInfo(const string & db_name, unsigned int info_type,uint32_t new_addr){
	RecordBlock* block_ptr = this->FindDatabaseBlock(db_name);
	uint32_t* database_info = (uint32_t*)(block_ptr->GetDataPtr(block_ptr->FindTupleIndex(db_name.c_str()), 1));
	database_info[info_type] = new_addr;
	buffer_manager.ReleaseBlock((Block* &)block_ptr);
}

void Catalog::CreateTable(const string & table_name, string* attr_name_list, DBenum* attr_type_list, int attr_num, int & key_index){
	if(!this->database_selected) throw DatabaseNotSelected("data base not selected!");
	// sort attr
	for (int i = 1; i < attr_num; i++) {
		string t1 = attr_name_list[i];
		DBenum t2 = attr_type_list[i];
		int j = i - 1;
		for (; j >= 0 && attr_name_list[j] > t1; j--){
			attr_name_list[j + 1] = attr_name_list[j];
			attr_type_list[j + 1] = attr_type_list[j];
		}
		attr_name_list[j + 1] = t1;
		attr_type_list[j + 1] = t2;
		if(key_index == i) key_index = j + 1;
		else if(key_index > j && key_index < i) key_index++;
	}

/* find the block where `table_name` should insert in */
	TypedIndexManager<ConstChar<32> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->table_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()));

	uint32_t table_block_addr;
	if(result_ptr){
		BPlusNode<ConstChar<32> >* leaf_node = static_cast<BPlusNode<ConstChar<32> >*>(result_ptr->node);
		ConstChar<32> & key = leaf_node->data()[result_ptr->index];
		if(key ==  ConstChar<32>(table_name.c_str())){
			buffer_manager.ReleaseBlock((Block* &)index_tree_root);
			throw DuplicatedTableName(table_name.c_str());
		}
		else{
			if (result_ptr->index != 0) {
				result_ptr->index--;
			}
			table_block_addr = leaf_node->ptrs()[result_ptr->index + 1];
		}
	}
	else{
	// first record to insert
		table_block_addr = this->table_data_addr;
	}

/* do real insertion */
	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));
	Block* new_table_block = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	uint32_t table_addr = new_table_block->BlockIndex();
	uint32_t index_addr = 0;
	buffer_manager.ReleaseBlock(new_table_block);

	// create the first index block for primary key
	int real_key = key_index < 0 ? 0 : key_index;
	Block* new_index_block = buffer_manager.CreateBlock();
	this->InitBPIndexRoot(new_index_block, attr_type_list[real_key]);
	index_addr = new_index_block->BlockIndex();
	buffer_manager.ReleaseBlock(new_index_block);

	try {
		table_block_ptr->InsertTable(table_name.c_str(), table_addr, index_addr, (uint8_t)attr_num, (uint8_t)key_index);
	}
	catch (const DuplicatedTableName & e) {
		buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
		buffer_manager.ReleaseBlock(index_tree_root);
		throw e;
	}
	for(int i = 0; i < attr_num; i++){
		table_block_ptr->InsertAttr(attr_name_list[i].c_str(),attr_type_list[i]);
	}
	table_block_ptr->is_dirty = true;

/* update index entry*/
	// calculate the size of the record
	short size = TableBlock::TABLE_RECORD_SIZE + TableBlock::ATTR_RECORD_SIZE * attr_num;
	if (!result_ptr) {
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()), this->table_data_addr);
	}
	else if (strcmp((char*)table_block_ptr->GetTableInfoPtr(0), table_name.c_str()) == 0) {
		index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()), table_block_addr);
	}
	if (size > table_block_ptr->EmptySize()) {
		TableBlock* new_block_ptr = this->SplitTableBlock(table_block_ptr);
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
							&ConstChar<32>((char*)new_block_ptr->GetTableInfoPtr(0)), new_block_ptr->BlockIndex());
		buffer_manager.ReleaseBlock((Block* &)new_block_ptr);
	}
	if (index_tree_root->BlockIndex() != this->table_index_addr) {
		this->UpdateDatabaseTableIndex(this->current_database_name, index_tree_root->BlockIndex());
		this->table_index_addr = index_tree_root->BlockIndex();
	}
	delete result_ptr;
	buffer_manager.ReleaseBlock(index_tree_root);
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

void Catalog::DeleteTable(const string & table_name){
	TableMeta* table_meta = this->GetTableMeta(table_name);
	if(table_meta->key_index >= 0){
		DBenum key_type = table_meta->attr_type_list[table_meta->key_index];
		Block* index_tree_root = buffer_manager.GetBlock(table_meta->primary_index_addr);
		TypedIndexManager<int> index_manager;
		index_manager.removeIndex(index_tree_root, BPTree);
		buffer_manager.ReleaseBlock(index_tree_root);
	}
	RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(table_meta->table_addr));
	while (data_block_ptr->NextBlockIndex()!= 0) {
		Block* next_block_ptr = buffer_manager.GetBlock(data_block_ptr->NextBlockIndex());
		data_block_ptr->NextBlockIndex() = next_block_ptr->NextBlockIndex();
		buffer_manager.DeleteBlock(next_block_ptr);
	}
	data_block_ptr->RecordNum() = 0;
	data_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)data_block_ptr);
}

void Catalog::DropTable(const string & table_name){
	if(!this->database_selected) throw DatabaseNotSelected("database not selected!");
/* delete data */
	this->DeleteTable(table_name);
/* search */
	TypedIndexManager<ConstChar<32> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->table_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()));

	uint32_t table_block_addr;
	if(result_ptr){
		BPlusNode<ConstChar<32> >* leaf_node = static_cast<BPlusNode<ConstChar<32> >*>(result_ptr->node);
		ConstChar<32> & key = leaf_node->data()[result_ptr->index];
		if(key == ConstChar<32>(table_name.c_str())){
			table_block_addr = leaf_node->ptrs()[result_ptr->index + 1];
		}
		else{
			if(result_ptr->index == 0){
				delete result_ptr;
				buffer_manager.ReleaseBlock(index_tree_root);
				throw TableNotFound(table_name.c_str());
			}
			else{
				table_block_addr = leaf_node->ptrs()[result_ptr->index];
				result_ptr->index--;
			}
		}		
	}
	else{
		delete result_ptr;
		buffer_manager.ReleaseBlock(index_tree_root);
		throw TableNotFound(table_name.c_str());
	}

/* real deletion */
	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));
	// find record
	int i = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(i < 0 || strcmp((char*)table_block_ptr->GetTableInfoPtr(i), table_name.c_str()) != 0){
		//not found
		delete result_ptr;
		buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
		buffer_manager.ReleaseBlock(index_tree_root);
		throw TableNotFound(table_name.c_str());
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
		index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
				&ConstChar<32>((char*)table_block_ptr->GetTableInfoPtr(0)), table_block_ptr->BlockIndex());	
	}
	else if(table_block_ptr->RecordNum() == 0){
		index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
		// the block would be empty remove it from link list
		if(table_block_ptr->PreBlockIndex() == 0 && table_block_ptr->NextBlockIndex() == 0){}
		else if(table_block_ptr->PreBlockIndex() == 0){
			this->UpdateDatabaseTableData(this->current_database_name, table_block_ptr->NextBlockIndex());
			this->table_data_addr = table_block_ptr->NextBlockIndex();
			Block* next_block_ptr = buffer_manager.GetBlock(table_block_ptr->NextBlockIndex());
			next_block_ptr->PreBlockIndex() = 0;
			next_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(next_block_ptr);
			buffer_manager.DeleteBlock((Block* &)table_block_ptr);
		}
		else if(table_block_ptr->NextBlockIndex() == 0){
			Block* pre_block_ptr = buffer_manager.GetBlock(table_block_ptr->PreBlockIndex());
			pre_block_ptr->NextBlockIndex() = 0;
			pre_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock((Block* &)table_block_ptr);
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
			buffer_manager.DeleteBlock((Block* &)table_block_ptr);
		}
	}
	if(index_tree_root->BlockIndex() != this->table_index_addr){
		this->UpdateDatabaseTableIndex(this->current_database_name, index_tree_root->BlockIndex());
		this->table_index_addr = index_tree_root->BlockIndex();
	}
	buffer_manager.ReleaseBlock(index_tree_root);
	if(table_block_ptr) {
		table_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
	}
	delete result_ptr;
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
	// split the old, always remove the last record to the new table block
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
	TypedIndexManager<ConstChar<32> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->table_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()));
	if(!result_ptr){
	// table_name not existed
		buffer_manager.ReleaseBlock((Block* &)index_tree_root);
		throw TableNotFound(table_name.c_str());
	}
	BPlusNode<ConstChar<32> >* leaf_node = static_cast<BPlusNode<ConstChar<32> >*>(result_ptr->node);
	ConstChar<32> & key = leaf_node->data()[result_ptr->index];
	uint32_t table_block_addr;
	if(key == ConstChar<32>(table_name.c_str())){
		table_block_addr = leaf_node->ptrs()[result_ptr->index + 1];
	}
	else if(result_ptr->index != 0){
		table_block_addr = leaf_node->ptrs()[result_ptr->index];
	}
	else{
	// table_name less than the smallest key in index
		buffer_manager.ReleaseBlock(index_tree_root);
		throw TableNotFound(table_name.c_str());
	}
	buffer_manager.ReleaseBlock(index_tree_root);
	delete result_ptr;
	return table_block_addr;
}


void Catalog::UpdateTablePrimaryIndex(const std::string & table_name, uint32_t new_addr){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));

	int row = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(row < 0 || strcmp(table_name.c_str(), (char*)table_block_ptr->GetTableInfoPtr(row)) != 0){
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
		throw TableNotFound(table_name.c_str());
	}
	*(uint32_t*)(table_block_ptr->GetTableInfoPtr(row) + 40) = new_addr;
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

void Catalog::UpdateTableDataAddr(const std::string & table_name, uint32_t new_addr){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));

	unsigned short row = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(strcmp(table_name.c_str(), (char*)table_block_ptr->GetTableInfoPtr(row)) != 0){
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
		throw TableNotFound(table_name.c_str());
	}
	*(uint32_t*)(table_block_ptr->GetTableInfoPtr(row) + 36) = new_addr;
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

TableMeta* Catalog::GetTableMeta(const string & table_name){
	uint32_t table_block_addr = this->FindTableBlock(table_name);
	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));
/*  load data to TableMeta structure*/
	uint16_t attr_addr = 0;
	uint8_t attr_num, key_index;
	TableMeta*  ret = new TableMeta(table_name);
	try{
		table_block_ptr->GetTableMeta(ret->table_name.c_str(), ret->table_addr, ret->primary_index_addr, attr_num,  attr_addr, key_index);
	}
	catch(const Exception & e){
		delete ret;
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
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
	}
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
	return ret;
}

void Catalog::CreateIndex(const string & index_name, const string & table_name, const string & attr_name){
	if(!this->database_selected) throw DatabaseNotSelected("database not selected");
	Block* block_get_by_index_name = NULL;
	try{
		block_get_by_index_name = this->FindIndexByName(index_name);
	}
	catch(const IndexNotFound &){}
	if(block_get_by_index_name){
		buffer_manager.ReleaseBlock(block_get_by_index_name);
		throw DuplicatedIndexName("duplicate index name : " + index_name);
	}

	TableMeta* table_meta = this->GetTableMeta(table_name);
	int secondary_key_index = -1;
	DBenum type;
	for (int i = 0; i < table_meta->attr_num; i++) {
		if (table_meta->attr_name_list[i] == attr_name) {
			secondary_key_index = i;
			type = table_meta->attr_type_list[i];
		}
	}
	if (secondary_key_index < 0) {
		delete table_meta;
		throw AttributeNotFound("attribute not found: " + attr_name);
	}
/* do finding */
	// mix table name and index key, the combination is distinct
	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);

	TypedIndexManager<ConstChar<33> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->index_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<33>(table_name_mix_key.c_str()));

	uint32_t index_block_addr;
	if(result_ptr){
		BPlusNode<ConstChar<33> >* leaf_node = static_cast<BPlusNode<ConstChar<33> >*>(result_ptr->node);
		ConstChar<33> & key = leaf_node->data()[result_ptr->index];
		if(key == ConstChar<33>(table_name_mix_key.c_str())){
			buffer_manager.ReleaseBlock(index_tree_root);
			throw DuplicatedIndex(table_name.c_str(), secondary_key_index);			
		}
		else{
			if(result_ptr->index == 0){
				index_block_addr = leaf_node->ptrs()[result_ptr->index + 1];
			}
			else{
				result_ptr->index--;
				index_block_addr =leaf_node->ptrs()[result_ptr->index];
			}
		}
	}
	else{
		index_block_addr = this->index_data_addr;
	}
				
/* do real insertion record */
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* index_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(index_block_addr));
	index_block_ptr->Format(type_list, 3, 0);

	Block* index_root = buffer_manager.CreateBlock();
	this->InitBPIndexRoot(index_root, type);
	IndexManager* index_manager_ptr = getIndexManager(type);
	RecordBlock* data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(table_meta->table_addr));
	while (true) {
		data_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);
		index_root = index_manager_ptr->insertEntry(index_root, BPTree, data_block_ptr->GetDataPtr(0, table_meta->key_index), data_block_ptr->BlockIndex());
		uint32_t next = data_block_ptr->BlockIndex();
		buffer_manager.ReleaseBlock((Block* &)data_block_ptr);
		if (next == 0) break;
		else data_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
	}
	uint32_t new_block_addr = index_root->BlockIndex();
	delete table_meta;
	delete index_manager_ptr;
	buffer_manager.ReleaseBlock((Block* &)data_block_ptr);
	buffer_manager.ReleaseBlock(index_root);

	const void* data_list[3];
	data_list[0] = table_name_mix_key.c_str();
	data_list[1] = index_name.c_str();
	data_list[2] = &new_block_addr;
	index_block_ptr->InsertTuple(data_list);

/* update index */
	if(!result_ptr){
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
						&ConstChar<33>(table_name_mix_key.c_str()), index_block_ptr->BlockIndex());
	}
	else if(strcmp(table_name_mix_key.c_str(), (char*)index_block_ptr->GetDataPtr(0, 0)) == 0){
		index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
						&ConstChar<33>(table_name_mix_key.c_str()), index_block_ptr->BlockIndex());
	}
	// split if needed
	if(!index_block_ptr->CheckEmptySpace()){
		RecordBlock* new_block_ptr = this->SplitRecordBlock(index_block_ptr, type_list, 3, 0);
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
						&ConstChar<33>((char*)new_block_ptr->GetDataPtr(0,0)), new_block_ptr->BlockIndex());
		buffer_manager.ReleaseBlock((Block* &)new_block_ptr);
	}

	delete result_ptr;
	if(index_tree_root->BlockIndex() != this->index_index_addr){
		this->UpdateDatabaseIndexIndex(this->current_database_name, index_tree_root->BlockIndex());
		this->index_index_addr = index_tree_root->BlockIndex();
	}
	buffer_manager.ReleaseBlock((Block* &)index_tree_root);
	index_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)index_block_ptr);
}

RecordBlock* Catalog::FindIndexByName(const string & index_name){
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->index_data_addr));
	index_data_ptr->Format(type_list, 3, 0);

	while(true){
		for(unsigned int i = 0; i < index_data_ptr->RecordNum(); i++){
			if(strcmp((char*)index_data_ptr->GetDataPtr(i, 1), index_name.c_str()) == 0){
				return index_data_ptr;
			}
		}
		uint32_t next = index_data_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock((Block* &)index_data_ptr);
		if(next == 0) throw IndexNotFound();
		index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
		index_data_ptr->Format(type_list, 5, 0);
	}	
}

void Catalog::DropIndex(const string & index_name){
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->index_data_addr));
	index_data_ptr->Format(type_list, 3, 0);

	bool flag = false;
	uint32_t index_root_block;
	while(true){
		int record_num = index_data_ptr->RecordNum();
		for(int i = record_num - 1; i >=0; i--){
			if(strcmp((char*)index_data_ptr->GetDataPtr(i, 1), index_name.c_str()) == 0){
				index_root_block = *(uint32_t*)index_data_ptr->GetDataPtr(i, 2);
				if(i == 0 && index_data_ptr->RecordNum() > 1){
					//update primary index
					TypedIndexManager<ConstChar<33> > index_manager;
					Block* index_root = buffer_manager.GetBlock(this->index_index_addr);
					SearchResult* index_data_entry = index_manager.searchEntry(index_root, BPTree, index_data_ptr->GetDataPtr(0, 0));
					index_root = index_manager.removeEntry(index_root, BPTree, index_data_entry);
					index_root = index_manager.insertEntry(index_root, BPTree, index_data_ptr->GetDataPtr(1, 0), index_data_ptr->BlockIndex());
					buffer_manager.ReleaseBlock(index_root);
					delete index_data_entry;
				}
				index_data_ptr->RemoveTuple(i);
				flag = true;
				break;
			}
		}
		if(flag) break;
		uint32_t next = index_data_ptr->NextBlockIndex();
		buffer_manager.ReleaseBlock((Block* &)index_data_ptr);
		if(next == 0) throw IndexNotFound();
		index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
		index_data_ptr->Format(type_list, 3, 0);
	}

	TypedIndexManager<int> index_manager;
	index_manager.removeIndex(buffer_manager.GetBlock(index_root_block), BPTree);

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
			buffer_manager.DeleteBlock((Block* &)index_data_ptr);
		}
		else if(index_data_ptr->NextBlockIndex() == 0){
			Block* pre_block_ptr = buffer_manager.GetBlock(index_data_ptr->PreBlockIndex());
			pre_block_ptr->NextBlockIndex() = 0;
			pre_block_ptr->is_dirty = true;
			buffer_manager.ReleaseBlock(pre_block_ptr);
			buffer_manager.DeleteBlock((Block* &)index_data_ptr);
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
			buffer_manager.DeleteBlock((Block* &)index_data_ptr);
		}
	}
	if(index_data_ptr){
		index_data_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock((Block* &)index_data_ptr);
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
	TypedIndexManager<ConstChar<32> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->index_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<33>(table_name_mix_key.c_str()));
	if(!result_ptr){
		buffer_manager.ReleaseBlock(index_tree_root);
		throw IndexNotFound();
	}
	uint32_t record_block_addr;
	BPlusNode<ConstChar<33> >* leaf_node = static_cast<BPlusNode<ConstChar<33> >*>(result_ptr->node);
	if(leaf_node->data()[result_ptr->index] == ConstChar<33>(table_name_mix_key.c_str())){
		record_block_addr = leaf_node->ptrs()[result_ptr->index + 1];
	}
	else if(result_ptr->index != 0){
		record_block_addr = leaf_node->ptrs()[result_ptr->index];
	}
	else{
		buffer_manager.ReleaseBlock(index_tree_root);
		throw IndexNotFound();		
	}
	buffer_manager.ReleaseBlock(index_tree_root);
	delete result_ptr;
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
		buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
		throw IndexNotFound();
	}

	*(uint32_t*)record_block_ptr->GetDataPtr(i, 2) = new_addr;
	buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
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
		buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
		throw IndexNotFound();
	}
	ret = *(uint32_t*)record_block_ptr->GetDataPtr(i, 2);
	buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
	return ret;
}

void Catalog::InitBPIndexRoot(Block* root, DBenum type){
	IndexManager* index_manager_ptr;
	switch(type){
		case DB_TYPE_INT:
			{
				index_manager_ptr = new TypedIndexManager<int>();
			}
			break;
		case DB_TYPE_FLOAT:
			{
				index_manager_ptr = new TypedIndexManager<float>();
			}
			break;
		default:
			{
				unsigned int _str_len = (int)(type - DB_TYPE_CHAR);
				if (_str_len < 16) {
					index_manager_ptr = new TypedIndexManager<ConstChar<16> >();
				}
				else if (_str_len < 33) {
					index_manager_ptr = new TypedIndexManager<ConstChar<33> >();
				}
				else if (_str_len < 64) {
					index_manager_ptr = new TypedIndexManager<ConstChar<64> >();
				}
				else if (_str_len < 128) {
					index_manager_ptr = new TypedIndexManager<ConstChar<128> >();
				}
				else {
					index_manager_ptr = new TypedIndexManager<ConstChar<256> >();
				}
			}
			break;
	}
	index_manager_ptr->initRootBlock(root, BPTree);
	delete index_manager_ptr;
	root->is_dirty = true;
}