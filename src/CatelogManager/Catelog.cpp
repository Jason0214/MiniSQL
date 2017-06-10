#include "Catelog.h"
#include "../EXCEPTION.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"

using namespace std;

static BufferManager & buffer_manager = BufferManager::Instance();

Catelog::Catelog(){
	SchemaBlock* schema_ptr = dynamic_cast<SchemaBlock*>(buffer_manager.GetBlock(0));
	if(schema_ptr->DBMetaAddr() == 0){
		Block* db_block = buffer_manager.CreateBlock(DB_DATABASE_BLOCK);
		db_block->BlockType() = DB_DATABASE_BLOCK;
		schema_ptr->DBMetaAddr() = db_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		buffer_manager.ReleaseBlock(db_block);
	}
	this->database_block_addr = schema_ptr->DBMetaAddr();
	
	if(schema_ptr->UserMetaAddr() == 0){
		Block* user_block = buffer_manager.CreateBlock(DB_USER_BLOCK);
		user_block->BlockType() = DB_USER_BLOCK;
		schema_ptr->UserMetaAddr() = user_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		buffer_manager.ReleaseBlock(user_block);
	}
	this->user_block_addr = schema_ptr->UserMetaAddr();

	if(schema_ptr->IndexMetaAddr() == 0){
		Block* index_block = buffer_manager.CreateBlock(DB_INDEX_BLOCK);
		index_block->BlockType() = DB_INDEX_BLOCK;
		schema_ptr->IndexMetaAddr() = index_block->BlockIndex();
		schema_ptr->EmptyPtr() += 4;
		buffer_manager.ReleaseBlock(index_block);
	}	
	this->index_block_addr = schema_ptr->IndexMetaAddr();

	this->table_data_addr = 0;
	this->table_index_addr = 0;
	this->database_selected = false;
}

void Catelog::CreateDatabase(const string & db_name){
	RecordBlock* database_block_ptr = dynamic_cast<RecordBlock*> (buffer_manager.GetBlock(this->database_block_addr));
	while(!database_block_ptr->CheckEmptySpace()){
		uint32_t next_block_index = database_block_ptr->BlockIndex();		
		RecordBlock* next_block_ptr = NULL;
		if(next_block_index == 0){
			next_block_ptr = dynamic_cast <RecordBlock*> (buffer_manager.CreateBlock(DB_RECORD_BLOCK));
			database_block_ptr->NextBlockIndex() = next_block_ptr->BlockIndex();
		}
		buffer_manager.ReleaseBlock((Block* &)database_block_ptr);
		database_block_ptr = next_block_ptr;
	}

	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_VARCHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;

	const void* data_list[3];
	Block* table_block_ptr = buffer_manager.CreateBlock(DB_TABLE_BLOCK);
	Block* index_block_ptr = buffer_manager.CreateBlock();
	data_list[0] = db_name.c_str();
	data_list[1] = &table_block_ptr->BlockIndex();
	data_list[2] = &index_block_ptr->BlockIndex();

	database_block_ptr->Format(type_list, 3, 0);
	database_block_ptr->InsertTuple(data_list);
	buffer_manager.ReleaseBlock(table_block_ptr);
	buffer_manager.ReleaseBlock(index_block_ptr);
	buffer_manager.ReleaseBlock((Block* &)database_block_ptr);
}

void Catelog::UseDatabase(const string & db_name){
	RecordBlock* db_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(this->database_block_addr));
	DBenum type_list[3];
	type_list[0] = (DBenum)(DB_TYPE_VARCHAR + 31);
	type_list[1] = DB_TYPE_INT;
	type_list[2] = DB_TYPE_INT;
	db_block_ptr->Format(type_list, 3, 0);

	while(db_block_ptr){
		for(unsigned int i = 0; i < db_block_ptr->RecordNum(); i++){
			if(strcmp((char*)db_block_ptr->GetDataPtr(i, 0),db_name.c_str()) == 0){
				this->table_data_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 1);
				this->table_index_addr = *(uint32_t*)db_block_ptr->GetDataPtr(i, 2);
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

void Catelog::CreateTable(string & table_name, string* attr_name_list, DBenum* attr_type_list, unsigned int attr_num, int key_index){
	BPlusTree< , >* tree_root = static_cast<BPlusTree< , >*>(buffer_manager.GetBlock(this->table_index_addr));	
	SearchResult* result_ptr = tree_root.search(table_name.c_str());
	TableBlock* table_block_ptr = NULL;
	short size = TableBlock::TABLE_RECORD_SIZE + TableBlock::ATTR_RECORD_SIZE * attr_num;
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
					do{
						memcpy(_table_name, table_block_ptr->GetTableName(table_block_ptr->RecordNum()-1), 32);
						table_block_ptr->GetTableMeta(_table_name, _table_addr, _index_addr, _attr_num, _attr_addr, _key_index);
						new_block_ptr->InsertTable(_table_name, _table_addr, _index_addr, _attr_num, _key_index);
						for(unsigned int i = 0; i < _attr_num; i++){
							table_block_ptr->GetAttrMeta(_attr_name, _attr_type, _attr_addr);
							_attr_addr += TableBlock::ATTR_RECORD_SIZE;
							new_block_ptr->InsertAttr(_attr_name,_attr_type);
						}
						table_block_ptr->DropTable(_table_name);
					}while(size > table_block_ptr->EmptySize());
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
		tree_root->insert(table_name, this->table_data_addr);
		table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(this->table_data_addr));
	}
	buffer_manager.ReleaseBlock((Block* &) tree_root);	

	Block* new_table_block = buffer_manager.CreateBlock(DB_TABLE_BLOCK);
	uint32_t table_addr = new_table_block->BlockIndex();
	buffer_manager.ReleaseBlock(new_table_block);
	Block* new_index_block = buffer_manager.CreateBlock();
	uint32_t index_addr = new_table_block->BlockIndex();
	buffer_manager.ReleaseBlock(new_index_block);
	table_block_ptr->InsertTable(table_name.c_str(), table_addr, index_addr, attr_num, key_index);
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->InsertAttr(attr_name_list[i].c_str(),attr_type_list[i]);
	}
	buffer_manager.ReleaseBlock((Block* &)table_block_ptr);
}

void Catelog::GetTableAttr(string & table_name, uint32_t & table_addr, uint32_t & index_addr, 
			string* attr_name_list, DBenum* attr_type_list, unsigned int & attr_num, unsigned int & key_index){
	BPlusTree< , >* tree_root = static_cast<BPlusTree< , >*>(buffer_manager.GetBlock(this->table_index_addr));	
	SearchResult* result_ptr = tree_root.search(table_name.c_str());
	if(!result_ptr){
		buffer_manager.ReleaseBlock((Block* &)tree_root);
		throw TableNotFound(table_name.c_str());
	}
	char* key = result_ptr->node->data()[result_ptr->index];
	uint32_t table_block_addr;
	if(strcmp(key, table_name.c_str()) == 0){
		table_block_addr = result_ptr->node->ptrs()[result_ptr->index];
	}
	else if(result_ptr->index != 0){
		table_block_addr = result_ptr->node->ptrs()[result_ptr->index - 1];
	}
	else{
		buffer_manager.ReleaseBlock((Block* &)tree_root);
		throw TableNotFound(table_name.c_str());
	}
	buffer_manager.ReleaseBlock((Block* &)tree_root);

	TableBlock* table_block_ptr = dynamic_cast<TableBlock*>(buffer_manager.GetBlock(table_block_addr));
	uint16_t attr_addr;
	try{
		table_block_ptr->GetTableMeta(table_name.c_str(),table_addr, index_addr, (uint8_t &)attr_num, attr_addr, (uint8_t &)key_index);
	}
	catch(const Exception & e){
		buffer_manager.ReleaseBlock((Block* &) table_block_ptr);
		throw e;
	}
	char buf[32];
	attr_name_list = new string[attr_num];
	attr_type_list = new DBenum[attr_num];
	for(unsigned int i = 0; i < attr_num; i++){
		table_block_ptr->GetAttrMeta(buf, attr_type_list[i], attr_addr);
		attr_addr += TableBlock::ATTR_RECORD_SIZE;
		attr_name_list[i] = string(buf);
	}
}

void Catelog::CreateIndex(){

}

void Catelog::GetIndex(){

}