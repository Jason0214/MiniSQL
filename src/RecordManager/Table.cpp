#include "Table.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"
#include "../CatalogManager/Catalog.h"

using namespace std;

static Catalog & catalog = Catalog::Instance();
static BufferManager & buffer_manager = BufferManager::Instance();
static IndexManager & index_manager = IndexManager::Instance();

// new table create from table already stored on disc, matrialized table
Table::Table(const std::string & table_name):
    table_flag(DB_MATERIALIZED),
    table_name(table_name)
    {
    TableMeta* table_meta = catalog.GetTableMeta(table_name);
    this->attr_num = table_meta->attr_num;
    this->attr_type = new DBenum[this->attr_num];
    memcpy(this->attr_type, table_meta->attr_type_list, sizeof(DBenum) * this->attr_num);
    this->attr_name = new string[this->attr_num];
    for(int i = 0; i < this->attr_num; i++){
        this->attr_name[i] = table_meta->attr_name_list[i];
    }
    this->key_index = table_meta->key_index;
    this->is_primary_key = table_meta->is_primary_key;
    this->index_addr = table_meta->primary_index_addr;
    this->data_addr = table_meta->table_addr;
    delete table_meta;
}

// new table create from a base table through either `select` or `project` 
// which will be specifed by parameter `op`.
Table::Table(const std::string & table_name, 
    const Table & based_table, 
    const AttrAlias & attr_name_map, 
    TableImplement op)
    :table_flag(DB_TEMPORAL),
    table_name(table_name){
    if(op == TABLE_SELECT){
        this->attr_num = based_table.attr_num;
        this->attr_type = new DBenum[this->attr_num];
        memcpy(this->attr_type, based_table.attr_type, sizeof(DBenum) * this->attr_num);
        this->attr_name = new string[this->attr_num];
        for(int i = 0; i < this->attr_num; i++){
            AttrAlias::const_iterator temp = attr_name_map.find(based_table.attr_name[i]);
            if(temp == attr_name_map.end()){
                this->attr_name[i] = based_table.attr_name[i];
            }
            else{
                this->attr_name[i] = temp->second;
            }
        }
    }
    else{
        
    }
}


// new table created from two base table, through either 'join' or
// `natural join`which sepecifed by `op`
Table::Table(const std::string & table_name, 
    const Table & based_table1, 
    const AttrAlias & attr_name_map1, 
    const Table & based_table2, 
    const AttrAlias & attr_name_map2,
    TableImplement op)
    :table_flag(DB_TEMPORAL),
    table_name(table_name){

}

pair<TableIterator*, TableIterator*> Table::PrimaryIndexFilter(const string & op, const void* value){
    if(this->table_flag == DB_TEMPORAL){
        TupleKey map_key = TupleKey(value, this->attr_type[this->key_index]);
        if(op == "<"){
            map<TupleKey,Tuple>::iterator map_iterator = ++this->table_data.lower_bound(map_key);
			TemporalTable_Iterator* iterator =  new TemporalTable_Iterator(map_iterator);
            return make_pair(this->begin(), iterator);
        }
        else if(op == ">"){
            map<TupleKey,Tuple>::iterator map_iterator = this->table_data.upper_bound(map_key);
			TemporalTable_Iterator* iterator = new TemporalTable_Iterator(map_iterator);
            return make_pair(iterator, this->end());
        }
        else if(op == "<="){
            map<TupleKey,Tuple>::iterator map_iterator = this->table_data.upper_bound(map_key);
			TemporalTable_Iterator* iterator = new TemporalTable_Iterator(map_iterator);
            return make_pair(this->begin(), iterator);           
        }
        else if(op == ">="){
            map<TupleKey,Tuple>::iterator map_iterator = ++this->table_data.lower_bound(map_key);
			TemporalTable_Iterator* iterator = new TemporalTable_Iterator(map_iterator);
            return make_pair(iterator, this->end());                
        }
        else if(op == "=="){
            map<TupleKey,Tuple>::iterator map_iterator = this->table_data.upper_bound(map_key);
			TemporalTable_Iterator* iterator_end = new TemporalTable_Iterator(map_iterator);
            map_iterator = ++this->table_data.lower_bound(map_key);
			TemporalTable_Iterator* iterator_begin = new TemporalTable_Iterator(map_iterator);
            return make_pair(iterator_begin, iterator_end);
        }
        else if(op == "!="){
            return make_pair(this->begin(), this->end());
        }
    }
    else{
        uint32_t begin_addr, end_addr;
        Table::BlockFilter(op, value, &begin_addr, &end_addr);
        MaterializedTable_Iterator* iterator_begin 
            = new MaterializedTable_Iterator(begin_addr, 0, this->attr_num, this->attr_type, this->key_index);
        MaterializedTable_Iterator* iterator_end 
            = new MaterializedTable_Iterator(end_addr, 0, this->attr_num, this->attr_type, this->key_index);
        return make_pair(iterator_begin, iterator_end);
    }
}

void Table::materialized_InsertTuple(const void** tuple_data){
    uint32_t record_block_addr;
    /* do finding the tuple key through index */
    DBenum key_type = this->attr_type[this->key_index];
    uint32_t index_root = this->index_addr;
    AutoPtr<SearchResult> result_ptr(index_manager.searchEntry(DB_BPTREE_INDEX, index_root,
                         key_type, tuple_data[this->key_index]));
    if (result_ptr.raw_ptr) {
        if (compare(result_ptr->node->getKey(result_ptr->index),
                        tuple_data[this->key_index], key_type) == 0) {
            if (this->is_primary_key) {
				throw DuplicatedPrimaryKey("Duplicated Primary Key");
            }
            else {
                record_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
            }
        }
        else {
            if (result_ptr->index != 0) {
				result_ptr->index--;
            } 
            record_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
        }
    }
    else {
        record_block_addr = this->data_addr;
    }

    // do real insert
    BlockPtr<RecordBlock> record_block_ptr(dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(record_block_addr)));
    record_block_ptr->Format(this->attr_type, this->attr_num, this->key_index);

    if (this->is_primary_key) {
        int i = record_block_ptr->FindTupleIndex(tuple_data[this->key_index]);
        if (i >= 0 && i < record_block_ptr->RecordNum() &&
            compare(tuple_data[this->key_index], record_block_ptr->GetDataPtr(i, this->key_index), key_type) == 0) {
			throw DuplicatedPrimaryKey("Duplicated Primary Key");
        }
        record_block_ptr->InsertTupleByIndex(tuple_data, i >= 0 ? i : 0);
    }
    else {
        record_block_ptr->InsertTuple(tuple_data);
    }

    // update index
    if (!result_ptr.raw_ptr) {
        index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
    }
    else {
        index_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_root, key_type, result_ptr.raw_ptr);
        index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
    }
    //check space, split if needed
    if (!record_block_ptr->CheckEmptySpace()) {
        RecordBlock* new_block_ptr = catalog.SplitRecordBlock(record_block_ptr.raw_ptr, this->attr_type, this->attr_num, this->key_index);
        index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, new_block_ptr->GetDataPtr(0, this->key_index), new_block_ptr->BlockIndex());
        buffer_manager.ReleaseBlock(new_block_ptr);
    }
    // update index root
    if (index_root != this->index_addr) {
        catalog.UpdateTablePrimaryIndex(this->table_name, index_root);
        this->index_addr = index_root;
    }
    record_block_ptr->is_dirty = true;
}

void Table::temporal_InsertTuple(const void** tuple_data){
    Tuple new_tuple = Tuple(tuple_data, this->attr_num, this->attr_type);
    TupleKey new_tuple_key = TupleKey(new_tuple[this->key_index], this->attr_type[this->key_index]);
    this->table_data.insert(make_pair(new_tuple_key, new_tuple));
}

void Table::BlockFilter(const std::string & op,
                        const void* value,
                        uint32_t* begin_block_addr,
                        uint32_t* end_block_addr){
    uint32_t index_root = index_addr;
	DBenum key_type = this->attr_type[this->key_index];
    uint32_t target_block_addr;
    AutoPtr<SearchResult> result_ptr(index_manager.searchEntry(DB_BPTREE_INDEX, index_root, key_type, value));
    if (compare(result_ptr->node->getKey(result_ptr->index), value, key_type) == 0) {
        target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
    }
    else {
        if (result_ptr->index != 0) {
			result_ptr->index--;
        }
        target_block_addr = result_ptr->node->addrs()[result_ptr->index + 1];
    }

    /* deletion starts here */
    BlockPtr<RecordBlock> target_block_ptr(dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(target_block_addr)));
    target_block_ptr->Format(this->attr_type, this->attr_num, this->key_index);

    // find the begin and end of tuple traversal
    if (op == "<") {
        *begin_block_addr = this->data_addr;
        *end_block_addr = target_block_ptr->NextBlockIndex();
    }
    else if (op == "<=") {
        *begin_block_addr = this->data_addr;
        *end_block_addr = target_block_ptr->NextBlockIndex();
        uint32_t next_block_addr = target_block_ptr->NextBlockIndex();
        while (next_block_addr != 0) {
            RecordBlock* block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(next_block_addr));
            block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
            if (compare(block_ptr->GetDataPtr(0, this->key_index), value, key_type) > 0) {
                *end_block_addr = block_ptr->BlockIndex();
                buffer_manager.ReleaseBlock(block_ptr);
                break;
            }
            next_block_addr = block_ptr->NextBlockIndex();
            buffer_manager.ReleaseBlock(block_ptr);
        }
    }
    else if (op == ">=") {
        *begin_block_addr = target_block_ptr->BlockIndex();
        *end_block_addr = 0;
    }
    else if (op == ">") {
        *begin_block_addr = target_block_ptr->BlockIndex();
        *end_block_addr = 0;
    }
    else if (op == "=") {
        *begin_block_addr = target_block_ptr->BlockIndex();
        *end_block_addr = 0;
        uint32_t next_block_addr = target_block_ptr->NextBlockIndex();
        while (next_block_addr != 0) {
            RecordBlock* block_ptr = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(target_block_ptr->NextBlockIndex()));
            block_ptr->Format(this->attr_type, this->attr_num, this->key_index);
            if (compare(block_ptr->GetDataPtr(0, this->key_index), value, key_type) > 0) {
                *end_block_addr = block_ptr->PreBlockIndex();
                buffer_manager.ReleaseBlock(block_ptr);
                break;
            }
            next_block_addr = block_ptr->NextBlockIndex();
            buffer_manager.ReleaseBlock(block_ptr);
        }
    }
    else if(op == "<>"){
        *begin_block_addr = data_addr;
        *end_block_addr = 0;
    }
}