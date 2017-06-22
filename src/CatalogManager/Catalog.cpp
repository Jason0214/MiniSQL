#include "Catalog.h"
#include "../EXCEPTION.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"

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
		buffer_manager.ReleaseBlock(db_block);
	}
	this->database_block_addr = schema_ptr->DBMetaAddr();
	
	if(schema_ptr->UserMetaAddr() == 0){
		Block* user_block = buffer_manager.CreateBlock(DB_USER_BLOCK);
		user_block->BlockType() = DB_USER_BLOCK;
		user_block->is_dirty = true;
		schema_ptr->UserMetaAddr() = user_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		buffer_manager.ReleaseBlock(user_block);
	}
	this->user_block_addr = schema_ptr->UserMetaAddr();

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
	}
	const void* data_list[3];
	Block* table_data_ptr = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	Block* table_index_ptr = buffer_manager.CreateBlock();
	Block* index_data_ptr = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	Block* index_index_ptr = buffer_manager.CreateBlock();

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

void Catalog::UseDatabase(const string & db_name){
	RecordBlock* db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->database_block_addr));
	DBenum type_list[5];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	type_list[3] = DB_TYPE_INT;
	type_list[4] = DB_TYPE_INT;
	db_block_ptr->Format(type_list, 5, 0);

	while(db_block_ptr){
		for(unsigned int i = 0; i < db_block_ptr->RecordNum(); i++){
			if(strcmp((char*)db_block_ptr->GetDataPtr(i, 0),db_name.c_str()) == 0){
				this->table_data_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 1);
				this->table_index_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 2);
				this->index_data_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 3);
				this->index_index_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 4);
				this->database_selected = true;
				buffer_manager.ReleaseBlock((Block* &)db_block_ptr);
				return;
			}
		}
		uint32_t next = db_block_ptr->NextBlockIndex();		
		buffer_manager.ReleaseBlock((Block* &)db_block_ptr);
		if(next == 0){
			throw DatabaseNotFound();
		}
		db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next));
	}
}

void Catalog::CreateTable(const string & table_name, string* attr_name_list, DBenum* attr_type_list, int attr_num, int key_index){
	if (!this->database_selected) throw DatabaseNotSelected();
	// get index
	TypedIndexManager<ConstChar<32>> index_manager;
	Block* index_block = buffer_manager.GetBlock(this->table_index_addr);
	SearchResult* result_ptr = index_manager.searchEntry(index_block, BPTree, 32, table_name.c_str());
	// calculate the size of the record
	short size = TableBlock::TABLE_RECORD_SIZE + TableBlock::ATTR_RECORD_SIZE * attr_num;
	TableBlock* table_block_ptr = NULL;
	if(result_ptr){
		char* key = result_ptr->node->data()[result_ptr->index];
		if(strcmp(key, table_name.c_str()) == 0){
			buffer_manager.ReleaseBlock((Block* &) tree_root);
			throw DuplicatedTableName(table_name.c_str());
		}
		else{
			if(result_ptr->index == 0){ //new key smaller than all the keys
				uint32_t table_block_addr = result_ptr->node->ptrs()[result_ptr->index];
				table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_data_addr));
				if(size > table_block_ptr->EmptySize()){
				// the current record block is full, create the new one and split the old
					TableBlock* new_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.CreateBlock(DB_TABLE_BLOCK));
					new_block_ptr->NextBlockIndex() = table_block_ptr->NextBlockIndex();
					table_block_ptr->NextBlockIndex() = new_block_ptr->BlockIndex();
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
						memcpy(_table_name, table_block_ptr->GetTableName(table_block_ptr->RecordNum()-1), 32);
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
					// update B+ tree index
					tree_root->remove(table_block_ptr->GetTableName(0), table_block_addr);
					tree_root->insert(table_name.c_str(), table_block_addr);
					tree_root->insert(new_block_ptr->GetTableName(0), new_block_ptr->BlockIndex());
					buffer_manager.ReleaseBlock((Block* &)new_block_ptr);
				}
				else{
					tree_root.remove(table_block_ptr->GetTableName(0), table_block_addr);
					tree_root.insert(table_name.c_str(), table_block_addr);
				}
			}
			else{
				uint32_t table_block_addr = result_ptr->node->ptrs()[result_ptr->index - 1];
				table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_data_addr));
			}
		}
	}
	else{
	// current record will be the first record
		tree_root->insert(table_name, this->table_data_addr);
		table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(this->table_data_addr));
	}
	buffer_manager.ReleaseBlock((Block* &) tree_root);

	Block* new_table_block = buffer_manager.CreateBlock(DB_TABLE_BLOCK);
	uint32_t table_addr = new_table_block->BlockIndex();
	uint32_t index_addr = 0;
	buffer_manager.ReleaseBlock(new_table_block);
	if(key_index == -1){
	// if no primary key specified
		this->CreateIndex(table_name, 0);
	}
	else{
	// create the first index block for primary key
		Block* new_index_block = buffer_manager.CreateBlock();
		index_addr = new_table_block->BlockIndex();		
		buffer_manager.ReleaseBlock(new_index_block);
	}
	table_block_ptr->InsertTable(table_name.c_str(), table_addr, index_addr, (uint8_t)attr_num, (uint8_t)key_index);
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->InsertAttr(attr_name_list[i].c_str(),attr_type_list[i]);
	}
	table_block_ptr->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

void Catalog::GetTableAttr(TableMeta & table_meta){
	if(!this->database_selected) throw DatabaseNotSelected();
	BPlusTree<ConstChar>* tree_root = static_cast<BPlusTree< , >*>(buffer_manager.GetBlock(this->table_index_addr));	
	SearchResult* result_ptr = tree_root.search(table_meta.table_name.c_str());
	if(!result_ptr){
	// table_name not existed
		buffer_manager.ReleaseBlock((Block* &)tree_root);
		throw TableNotFound(table_meta.table_name.c_str());
	}
	char* key = result_ptr->node->data()[result_ptr->index];
	uint32_t table_block_addr;
	if(strcmp(key, table_meta.table_name.c_str()) == 0){
		table_block_addr = result_ptr->node->ptrs()[result_ptr->index];
	}
	else if(result_ptr->index != 0){
		table_block_addr = result_ptr->node->ptrs()[result_ptr->index - 1];
	}
	else{
	// table_name less than the smallest key in index
		buffer_manager.ReleaseBlock((Block* &)tree_root);
		throw TableNotFound(table_meta.table_name.c_str());
	}
	buffer_manager.ReleaseBlock((Block* &)tree_root);

	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));
	
	uint16_t attr_addr = 0;
	uint8_t attr_num, key_index;
	try{
		table_block_ptr->GetTableMeta(table_meta.table_name.c_str(), table_meta.table_addr, table_meta.primary_index_addr, attr_num,  attr_addr, key_index);
	}
	catch(const Exception & e){
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
		throw e;
	}
	table_meta.attr_num = (int8_t)attr_num;
	table_meta.primay_key_index = (int8_t)key_index;
	char buf[32];
	table_meta.attr_name_list = new string[attr_num];
	table_meta.attr_type_list = new DBenum[attr_num];
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->GetAttrMeta(buf, table_meta.attr_type_list[i], attr_addr);
		attr_addr += TableBlock::ATTR_RECORD_SIZE;
		table_meta.attr_name_list[i] = string(buf);
	}
}

void Catalog::CreateIndex(const string & table_name, int8_t secondary_key_index){
	if(!this->database_selected) throw DatabaseNotSelected();	
	
	RecordBlock* index_block = ;// TODO

	DBenum type_list[2];
	type_list[0] = (DBenum)(DB_TYPE_CHAR + 32);
	type_list[1] = DB_TYPE_INT;
	index_block->Format(type_list, 2, 0);

	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);
	Block* new_block_ptr = buffer_manager.CreateBlock(DB_RECORD_BLOCK);
	uint32_t new_block_addr = new_block_ptr->BlockIndex();
	buffer_manager.ReleaseBlock(new_block_ptr);

	const void* data_list[2];
	data_list[0] = table_name_mix_key.c_str();
	data_list[1] = &new_block_addr;

	// update B+ tree	
	index_block->is_dirty = true;
	buffer_manager.ReleaseBlock((Block* &)index_block);
}

uint32_t Catalog::GetIndex(const string & table_name, int8_t secondary_key_index){
	// get B+ tree

	string table_name_mix_key = table_name;
	table_name_mix_key.append((char*)&secondary_key_index);
}