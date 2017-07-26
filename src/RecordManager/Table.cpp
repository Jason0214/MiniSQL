#include "Table.h"
#include <pair>

using namespace std;

static Catalog & catalog = Catalog::Instance();
static BufferManager & buffer_manager = BufferManager::Instance();
static IndexManager & index_manager = IndexManager::Instance();

// new table create from table already stored on disc, matrialized table
Table::Table(const std::string & table_name):
    table_flag(DB_MATERIALIZED),
    table_name(table_name),
    {
    TableMeta* table_meta = BufferManager::Instance().GetTableMeta(table_name);
    this->attr_num = table_meta->attr_num;
    this->attr_type = new DBenum[this->attr_num];
    memcpy(this->attr_type, table_meta->attr_type, sizeof(DBenum) * this->attr_num);
    this->attr_name = new string[this->attr_num];
    for(int i = 0; i < this->attr_num; i++){
        this->attr_name[i] = table_meta->attr_name[i];
    }
    this->key_index = table_meta->key_index;
    this->is_primary_key = table_meta->is_primary_key;
    this->index_addr = table_meta->primary_index_addr;
    this->data_addr = table_addr;
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
        this->attr_name = new string[this->attr_name];
        for(int i = 0; i < this->attr_num; i++){
            AttrAlias::iterator temp = attr_name_map.find(based_table.attr_name[i]);
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
            map<TupleKey,Tuple> map_iterator = ++this->map.lower_bound(map_key);
            TemproalTable_iterator* iterator =  new TemproalTable_iterator(map_iterator);
            return make_pair(this->begin(), iterator);
        }
        else if(op == ">"){
            map<TupleKey,Tuple> map_iterator = this->map.upper_bound(map_key);
            TemproalTable_iterator* iterator = new TemproalTable_iterator(map_iterator);
            return make_pair(iterator, this->end());
        }
        else if(op == "<="){
            map<TupleKey,Tuple> map_iterator = this->map.upper_bound(map_key);
            TemproalTable_iterator* iterator = new TemproalTable_iterator(map_iterator);
            return make_pair(this->begin(), iterator);           
        }
        else if(op == ">="){
            map<TupleKey,Tuple> map_iterator = ++this->map.lower_bound(map_key);
            TemproalTable_iterator* iterator = new TemproalTable_iterator(map_iterator);
            return make_pair(iterator, this->end());                
        }
        else if(op == "=="){
            map<TupleKey,Tuple> map_iterator = this->map.upper_bound(map_key);
            TemproalTable_iterator* iterator_end = new TemproalTable_iterator(map_iterator);
            map_iterator = ++this->map.lower_bound(map_key);
            TemproalTable_iterator* iterator_begin = new TemproalTable_iterator(map_iterator);
            return make_pair(iterator_begin, iterator_end);
        }
        else if(op == "!="){
            return make_pair(this->begin(), this->end());
        }
    }
    else{
        uint32_t begin_addr, end_addr;
        Table::BlockFilter(op, value, this->index_addr, this->attr_type[this->key_index], &begin_addr, &end_addr);
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
    if (result_ptr) {
        if (compare(result_ptr->node->getKey(result_ptr->index),
                        tuple_data[this->key_index], key_type) == 0) {
            if (this->is_primary_key) {
                cout << "Duplicated Primary Key" << endl;
                cout << "end_result" << endl;
                Flush();
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
    BlockPtr<RecordBlock> record_block_ptr(dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(record_block_addr)));
    record_block_ptr->Format(table_meta->attr_type_list, table_meta->attr_num, table_meta->key_index);

    if (this->is_primary_key) {
        int i = record_block_ptr->FindTupleIndex(tuple_data[table_meta->key_index]);
        if (i >= 0 && i < record_block_ptr->RecordNum() &&
            compare(tuple_data[table_meta->key_index], record_block_ptr->GetDataPtr(i, table_meta->key_index), key_type) == 0) {
            cout << "Duplicated Primary key" << endl;
            cout << "end_result" << endl;
            Flush();
            return;
        }
        record_block_ptr->InsertTupleByIndex(tuple_data, i >= 0 ? i : 0);
    }
    else {
        record_block_ptr->InsertTuple(tuple_data);
    }

    // update index
    if (!result_ptr) {
        index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
    }
    else {
        index_root = index_manager.removeEntry(DB_BPTREE_INDEX, index_root, key_type, result_ptr);
        index_root = index_manager.insertEntry(DB_BPTREE_INDEX, index_root, key_type, record_block_ptr->GetDataPtr(0, this->key_index), record_block_addr);
    }
    //check space, split if needed
    if (!record_block_ptr->CheckEmptySpace()) {
        RecordBlock* new_block_ptr = catalog.SplitRecordBlock(record_block_ptr.raw_ptr, this->attr_type_list, this->attr_num, this->key_index);
        index_root = index_manager->insertEntry(DB_BPTREE_INDEX, index_root, key_type, new_block_ptr->GetDataPtr(0, this->key_index), new_block_ptr->BlockIndex());
        buffer_manager->ReleaseBlock(new_block_ptr);
    }
    // update index root
    if (index_root != this->index_addr) {
        catalog.UpdateTablePrimaryIndex(table_meta->table_name, index_root);
        this->index_addr = index_root;
    }
    record_block_ptr->is_dirty = true;
}

void Table::temporal_InsertTuple(const void** tuple_data){
    Tuple new_tuple = Tuple(tuple_data, this->attr_num, this->attr_type);
    TupleKey new_tuple_key = TupleKey(new_tuple[this->key_index], this->attr_type(this->key_index));
    this->table_data.insert(make_pair(new_tuple_key, new_tuple));
}

void Table::BlockFilter(const std::string & op,
                        const void* value,
                        DBenum key_type,
                        uint32_t index_addr,
                        uint32_t* begin_block,
                        uint32_t* end_block){

}