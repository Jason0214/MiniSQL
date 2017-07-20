#include "Table.h"

static Catalog & catalog = Catalog::Instance();
static BufferManager & buffer_manager = BufferManager::Instance();
static IndexManager & index_manager = IndexManager::Instance();

// new table create from table already stored on disc, matrialized table
Table::Table(const std::string & table_name):
	table_flag(DB_MATERIALIZED),
	cursor_block(NULL){
	TableMeta* table_meta = BufferManager::Instance().GetTableMeta(table_name);
	for(int i = 0; i < table_meta->attr_num; i++){
		this->attr_type[i] = table_meta->attr_type[i];
		this->attr_name[i] = table_meta->attr_name[i];
	}
	this->key_index = table_meta->key_index;
	this->is_primary_key = table_meta->is_primary_key;
	this->index_addr = table_meta->primary_index_addr;
	this->data_addr = table_addr;
	delete table_meta;
}

// new table create from a base table through either `select` or `project` 
//	which will be specifed by parameter `op`.
Table::Table(const std::string & table_name, 
	const Table & based_table, 
	const AttrAlias & attr_name_map, 
	TableImplement op)
	:table_flag(DB_TEMPORAL),
	cursor_block(NULL){

}


// new table created from two base table, through either 'join' or
// `natural join`which sepecifed by `op`
Table::Table(const std::string & table_name, 
	const Table & based_table1, 
	const AttrAlias & attr_name_map1, 
	const Table & based_table2, 
	const AttrAlias & attr_name_map2,
	TableImplement op)
	:table_flag(DB_TEMPORAL){

}

void Table::materialized_InsertTuple(const void** tuple_data){
	uint32_t record_block_addr;
	/* do finding the tuple key through index */
	DBenum key_type = this->attr_type[this->key_index];
	SearchResult* result_ptr = index_manager.searchEntry(DB_BPTREE_INDEX, this->index_addr,
						 key_type, tuple_data[this->key_index]);
	if (result_ptr) {
		if (compare(result_ptr->node->getKey(result_ptr->index),
				tuple_data[this->key_index], key_type) == 0) {
			if (this->is_primary_key) {
				cout << "Duplicated Primary Key" << endl;
				cout << "end_result" << endl;
				Flush();
				buffer_manager->ReleaseBlock(index_root);
				delete result_ptr;
				return;
			}
			else {
				record_block_addr = result_ptr->attrs[result_ptr->index];
			}
		}
		else {
			if (result_ptr->index == 0) {
				record_block_addr = result_ptr->addrs[result_ptr->index];
			}
			else {
				result_ptr->index--;
				record_block_addr = result_ptr->addrs[result_ptr->index];
			}
		}
	}
	else {
		record_block_addr = this->data_addr;
	}

	// do real insert
	RecordBlock* record_block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(record_block_addr));
	record_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);

	if (this->is_primary_key) {
		int i = record_block_ptr->Findtuple_dataIndex(tuple_data[table_meta->key_index]);
		if (i >= 0 && i < record_block_ptr->RecordNum() &&
			compare(tuple_data[table_meta->key_index], record_block_ptr->GetDataPtr(i, table_meta->key_index), key_type) == 0) {
			cout << "Duplicated Primary key" << endl;
			cout << "end_result" << endl;
			Flush();
			buffer_manager->ReleaseBlock(index_root);
			buffer_manager->ReleaseBlock((Block* &)record_block_ptr);
			delete result_ptr;
			delete index_manager;
			return;
		}
		record_block_ptr->Inserttuple_dataByIndex(tuple_data, i >= 0 ? i : 0);
	}
	else {
		record_block_ptr->Inserttuple_data(tuple_data);
	}

	// update index
	if (!result_ptr) {
		index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
	}
	else {
		index_manager.removeEntry(DB_BPTREE_INDEX, index_root, key_type, result_ptr);
		index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
	}
	//check space, split if needed
	if (!record_block_ptr->CheckEmptySpace()) {
		RecordBlock* new_block_ptr = catalog.SplitRecordBlock(record_block_ptr, this->attr_type_list,
									this->attr_num, this->key_index);
		index_manager->insertEntry(DB_BPTREE_INDEX, index_root, key_type, new_block_ptr->GetDataPtr(0, this->key_index), new_block_ptr->BlockIndex());
		buffer_manager->ReleaseBlock(new_block_ptr);
	}
	// update index root
	if (index_root->BlockIndex() != this->index_addr) {
		catalog.UpdateTablePrimaryIndex(table_meta->table_name, index_root->BlockIndex());
		this->index_addr = index_root->Block();
	}
	record_block_ptr->is_dirty = true;
	buffer_manager->ReleaseBlock(record_block_ptr);
	buffer_manager->ReleaseBlock(index_root);
	index_manager.destroySearchResult(result_ptr);
}
