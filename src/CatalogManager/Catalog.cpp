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

RecordResult* Catalog::FindDatabaseBlock(const string & db_name){
	DBenum type_list[5];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	type_list[3] = DB_TYPE_INT;
	type_list[4] = DB_TYPE_INT;
	RecordBlock* db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->database_block_addr));
	db_block_ptr->Format(type_list, 5, 0);

	RecordResult* ret = NULL;
	while(true){
		int i = db_block_ptr->FindTupleIndex(db_name.c_str());
		if(i >= 0 && strcmp((char*)db_block_ptr->GetDataPtr(i, 0), db_name.c_str()) == 0){
			ret = new RecordResult;
			ret->block_ptr = db_block_ptr;
			ret->index = i;
			break;
		}
		uint32_t next = db_block_ptr->NextBlockIndex();		
		buffer_manager.ReleaseBlock((Block* &)db_block_ptr);
		if(next == 0) throw DatabaseNotFound();
		db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
		db_block_ptr->Format(type_list, 5, 0);
	}
	buffer_manager.ReleaseBlock((Block* &)db_block_ptr);
	return ret;
}

void Catalog::UseDatabase(const string & db_name){
	RecordResult* result = this->FindDatabaseBlock(db_name);
	this->current_database_name = db_name;
	uint32_t* database_info = (uint32_t*)(result->block_ptr->GetDataPtr(result->index, 1));
	this->table_data_addr = database_info[0];
	this->table_index_addr = database_info[1];
	this->index_data_addr = database_info[2];
	this->index_index_addr = database_info[3];
	this->database_selected = true;
	buffer_manager.ReleaseBlock((Block* &)result->block_ptr);
	delete result;
}

void Catalog::UpdateDatabaseInfo(const string & db_name, unsigned int info_type,uint32_t new_addr){
	RecordResult* result = this->FindDatabaseBlock(db_name);
	uint32_t* database_info = (uint32_t*)(result->block_ptr->GetDataPtr(result->index, 1));
	database_info[info_type] = new_addr;
	buffer_manager.ReleaseBlock((Block* &)result->block_ptr);
	delete result;
}

void Catalog::CreateTable(const string & table_name, string* attr_name_list, DBenum* attr_type_list, int attr_num, int & key_index){
	if(!this->database_selected) throw DatabaseNotSelected();
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
	// calculate the size of the record
	short size = TableBlock::TABLE_RECORD_SIZE + TableBlock::ATTR_RECORD_SIZE * attr_num;
	TableBlock* table_block_ptr = NULL;
	if(result_ptr){
		BPlusNode<ConstChar<32> >* leaf_node = static_cast<BPlusNode<ConstChar<32> >*>(result_ptr->node);
		ConstChar<32> & key = leaf_node->data()[result_ptr->index];
		if(key ==  ConstChar<32>(table_name.c_str())){
			buffer_manager.ReleaseBlock((Block* &)index_tree_root);
			throw DuplicatedTableName(table_name.c_str());
		}
		else{
			if(result_ptr->index == 0){ 
			//new key smaller than all the keys
				table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index + 1]));
				// update B plus tree index
				index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr); // new key less than the Block[0], remove the index 0 entry		
				index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()), table_block_ptr->BlockIndex());
			}	
			else{
				table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index]));
			}
			if(size > table_block_ptr->EmptySize()){
				TableBlock* new_block_ptr = this->SplitTableBlock(table_block_ptr);
				index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
								&ConstChar<32>((char*)new_block_ptr->GetTableInfoPtr(0)), new_block_ptr->BlockIndex());
				buffer_manager.ReleaseBlock((Block* &)new_block_ptr);
			}
		}
	}
	else{
	// first record to insert
		index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()), this->table_data_addr);
		table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(this->table_data_addr));
	}

	// if the root of index tree changed, update it
	if(this->table_index_addr != index_tree_root->BlockIndex()){
		this->UpdateDatabaseTableIndex(this->current_database_name ,index_tree_root->BlockIndex());	
	}
	buffer_manager.ReleaseBlock(index_tree_root);
	delete result_ptr;

/* do real insertion */
	Block* new_table_block = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	uint32_t table_addr = new_table_block->BlockIndex();
	uint32_t index_addr = 0;
	buffer_manager.ReleaseBlock(new_table_block);
	if(key_index == -1){
	// if no primary key specified, create a secondary index
		this->CreateIndex("DEFAULT", table_name,  0, attr_type_list[0]);
	}
	else{
	// create the first index block for primary key
		Block* new_index_block = buffer_manager.CreateBlock();
		this->InitBPIndexRoot(new_index_block, attr_type_list[key_index]);
		index_addr = new_index_block->BlockIndex();
		buffer_manager.ReleaseBlock(new_index_block);
	}
	table_block_ptr->InsertTable(table_name.c_str(), table_addr, index_addr, (uint8_t)attr_num, (uint8_t)key_index);
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->InsertAttr(attr_name_list[i].c_str(),attr_type_list[i]);
	}
	table_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

void Catalog::DropTable(const string & table_name){
	TypedIndexManager<ConstChar<32> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->table_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<32>(table_name.c_str()));

	TableBlock* table_block_ptr = NULL;
	if(result_ptr){
		BPlusNode<ConstChar<32> >* leaf_node = static_cast<BPlusNode<ConstChar<32> >*>(result_ptr->node);
		ConstChar<32> & key = leaf_node->data()[result_ptr->index];
		if(key == ConstChar<32>(table_name.c_str())){
			table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index + 1]));
			index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
			if(table_block_ptr->RecordNum() > 1){
				index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
						&ConstChar<32>((char*)table_block_ptr->GetTableInfoPtr(1)), table_block_ptr->BlockIndex());
				table_block_ptr->DropTable(table_name.c_str());
			}
			else{
			// the block would be empty remove it from link list
				if(table_block_ptr->PreBlockIndex() == 0 && table_block_ptr->NextBlockIndex() == 0){
					table_block_ptr->DropTable(table_name.c_str());	
				}
				else if(table_block_ptr->PreBlockIndex() == 0){
					this->UpdateDatabaseTableData(this->current_database_name, table_block_ptr->NextBlockIndex());
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
		}
		else{
			if(result_ptr->index == 0){
				buffer_manager.ReleaseBlock(index_tree_root);
				throw TableNotFound(table_name.c_str());
			}
			else{
				table_block_ptr->is_dirty = true;
				table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index]));
				table_block_ptr->DropTable(table_name.c_str());
			}
		}		
	}
	else{
		buffer_manager.ReleaseBlock(index_tree_root);
		throw TableNotFound(table_name.c_str());
	}
	if(index_tree_root->BlockIndex() != this->table_index_addr){
		this->UpdateDatabaseTableIndex(this->current_database_name, index_tree_root->BlockIndex());
	}
	buffer_manager.ReleaseBlock(index_tree_root);

	if(table_block_ptr) {
		table_block_ptr->is_dirty = true;
		buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
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
	// split the old, always remove the last record to the new table block
	for(unsigned int i = 0; i < table_block_ptr->RecordNum()/2; i++){
		memcpy(_table_name, table_block_ptr->GetTableInfoPtr(table_block_ptr->RecordNum()-1), 32);
		table_block_ptr->GetTableMeta(_table_name, _table_addr, _index_addr, _attr_num, _attr_addr, _key_index);
		new_block_ptr->InsertTable(_table_name, _table_addr, _index_addr, _attr_num, _key_index);
		for(unsigned int j = 0; j < _attr_num; j++){
			table_block_ptr->GetAttrMeta(_attr_name, _attr_type, _attr_addr);
			_attr_addr += TableBlock::ATTR_RECORD_SIZE;
			new_block_ptr->InsertAttr(_attr_name,_attr_type);
		}
		table_block_ptr->DropTable(_table_name);
	}
	table_block_ptr->is_dirty = true;
	new_block_ptr->is_dirty = true;
	return new_block_ptr;
}

uint32_t Catalog::FindTableBlock(const std::string & table_name){
	if(!this->database_selected) throw DatabaseNotSelected();
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

	unsigned short row = table_block_ptr->FindRecordIndex(table_name.c_str());
	if(strcmp(table_name.c_str(), (char*)table_block_ptr->GetTableInfoPtr(row)) != 0){
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
	TableMeta*  ret = new TableMeta(table_name);	
	uint16_t attr_addr = 0;
	uint8_t attr_num, key_index;
	try{
		table_block_ptr->GetTableMeta(ret->table_name.c_str(), ret->table_addr, ret->primary_index_addr, attr_num,  attr_addr, key_index);
	}
	catch(const Exception & e){
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
		throw e;
	}
	ret->attr_num = (int8_t)attr_num;
	ret->primay_key_index = (int8_t)key_index;
	char buf[32];
	ret->attr_name_list = new string[attr_num];
	ret->attr_type_list = new DBenum[attr_num];
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->GetAttrMeta(buf, ret->attr_type_list[i], attr_addr);
		attr_addr -= TableBlock::ATTR_RECORD_SIZE;
		ret->attr_name_list[i] = string(buf);
	}
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
	return ret;
}

void Catalog::CreateIndex(const string & index_name, const string & table_name, int8_t secondary_key_index, DBenum type){
	if(!this->database_selected) throw DatabaseNotSelected();
/* do finding */
	// mix table name and index key, the combination is distinct	
	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);

	TypedIndexManager<ConstChar<33> > index_manager;
	Block* index_tree_root = buffer_manager.GetBlock(this->index_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_tree_root, BPTree, &ConstChar<33>(table_name_mix_key.c_str()));

	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* record_block_ptr = NULL;
	if(result_ptr){
		BPlusNode<ConstChar<33> >* leaf_node = static_cast<BPlusNode<ConstChar<33> >*>(result_ptr->node);
		ConstChar<33> & key = leaf_node->data()[result_ptr->index];
		if(key == ConstChar<33>(table_name_mix_key.c_str())){
			buffer_manager.ReleaseBlock(index_tree_root);
			throw DuplicatedIndex(table_name.c_str(), secondary_key_index);			
		}
		else{
			if(result_ptr->index == 0){
				record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index + 1]));
				index_tree_root = index_manager.removeEntry(index_tree_root, BPTree, result_ptr);
				index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
								&ConstChar<33>(table_name_mix_key.c_str()), record_block_ptr->BlockIndex());
			}
			else{
				record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(leaf_node->ptrs()[result_ptr->index]));				
			}
			record_block_ptr->Format(type_list, 3, 0);
			if(!record_block_ptr->CheckEmptySpace()){
				RecordBlock* new_block_ptr = this->SplitRecordBlock(record_block_ptr, type_list, 3, 0);
				index_tree_root = index_manager.insertEntry(index_tree_root, BPTree, 
								&ConstChar<33>((char*)new_block_ptr->GetDataPtr(0,0)), new_block_ptr->BlockIndex());
				buffer_manager.ReleaseBlock((Block* &)new_block_ptr);
			}
		}
	}
	else{
		index_manager.insertEntry(index_tree_root, BPTree, &ConstChar<33>(table_name_mix_key.c_str()), this->index_data_addr);
		record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->index_data_addr));
		record_block_ptr->Format(type_list, 3, 0);
	}
	if(index_tree_root->BlockIndex() != this->index_index_addr){
		this->UpdateDatabaseIndexIndex(this->current_database_name, index_tree_root->BlockIndex());
	}
	buffer_manager.ReleaseBlock((Block* &)index_tree_root);
	delete result_ptr;

/* do real insertion record */
	Block* new_block_ptr = buffer_manager.CreateBlock();
	this->InitBPIndexRoot(new_block_ptr, type);
	uint32_t new_block_addr = new_block_ptr->BlockIndex();
	buffer_manager.ReleaseBlock(new_block_ptr);

	const void* data_list[3];
	data_list[0] = table_name_mix_key.c_str();
	data_list[1] = index_name.c_str();
	data_list[2] = &new_block_addr;
	record_block_ptr->InsertTuple(data_list);
	record_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
}

void Catalog::DropIndex(const string & index_name){
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[2] = DB_TYPE_INT;

	RecordBlock* index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->index_data_addr));
	index_data_ptr->Format(type_list, 3, 0);

	bool flag = false;
	while(true){
		for(unsigned int i = 0; i < index_data_ptr->RecordNum(); i++){
			if(strcmp((char*)index_data_ptr->GetDataPtr(i, 1), index_name.c_str()) == 0){
				index_data_ptr->RemoveTuple(i);
				flag = true;
				break;
			}
		}
		if(flag) break;
		uint32_t next = index_data_ptr->NextBlockIndex();		
		buffer_manager.ReleaseBlock((Block* &)index_data_ptr);
		if(next == 0) throw IndexNotFound(index_name.c_str());
		index_data_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
		index_data_ptr->Format(type_list, 5, 0);
	}

	if(index_data_ptr->RecordNum() == 0){
		if(index_data_ptr->PreBlockIndex() == 0 && index_data_ptr->NextBlockIndex() == 0){
		}
		else if(index_data_ptr->PreBlockIndex() == 0){
			this->UpdateDatabaseIndexData(this->current_database_name, index_data_ptr->NextBlockIndex());
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
	RecordBlock* new_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.CreateBlock(DB_TABLE_BLOCK));
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
	origin_block_ptr->RecordNum() == half_of_records;
	new_block_ptr->is_dirty = true;
	origin_block_ptr->is_dirty = true;
	return new_block_ptr;
}

uint32_t Catalog::FindIndexBlock(const std::string & table_name_mix_key){
	if(!this->database_selected) throw DatabaseNotSelected();
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
		e.table_name = table_name;
		e.key_index = key_index;
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
		throw IndexNotFound(table_name, key_index);
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
		e.table_name = table_name;
		e.key_index = secondary_key_index;
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
		throw IndexNotFound(table_name, secondary_key_index);
	}
	ret = *(uint32_t*)record_block_ptr->GetDataPtr(i, 2);
	buffer_manager.ReleaseBlock((Block* &)record_block_ptr);
	return ret;
}

void Catalog::InitBPIndexRoot(Block* root, DBenum type){
	switch(type){
		case DB_TYPE_INT:
			{
				TypedIndexManager<int> index_manager;
				index_manager.initRootBlock(root, BPTree);
			}
			break;
		case DB_TYPE_FLOAT:
			{
				TypedIndexManager<float> index_manager;
				index_manager.initRootBlock(root, BPTree);
			}
			break;
		default:
			{
				unsigned int _str_len = (int)(type - DB_TYPE_CHAR);
				if (_str_len < 16) {
					TypedIndexManager<ConstChar<16> > index_manager;
					index_manager.initRootBlock(root, BPTree);
				}
				else if (_str_len < 33) {
					TypedIndexManager<ConstChar<33> > index_manager;
					index_manager.initRootBlock(root, BPTree);
				}
				else if (_str_len < 64) {
					TypedIndexManager<ConstChar<64> > index_manager;
					index_manager.initRootBlock(root, BPTree);
				}
				else if (_str_len < 128) {
					TypedIndexManager<ConstChar<128> > index_manager;
					index_manager.initRootBlock(root, BPTree);
				}
				else {
					TypedIndexManager<ConstChar<256> > index_manager;
					index_manager.initRootBlock(root, BPTree);
				}
			}
			break;
	}
	root->is_dirty = true;
}