#include "Table.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"
#include "../CatalogManager/Catalog.h"

using namespace std;

static BufferManager & buffer_manager = BufferManager::Instance();
static Catalog & catalog = Catalog::Instance();
static IndexManager & index_manager = IndexManager::Instance();

// new table create from table already stored on disc, matrialized table
Table::Table(const std::string & table_name):
    table_flag(DB_ONDISC_TABLE),
    table_name(table_name)
    {
    TableMeta* table_meta = catalog.GetTableMeta(table_name);
    this->attr_num = table_meta->attr_num;
    this->attr_type = new DBenum[this->attr_num];
    memcpy(this->attr_type, table_meta->attr_type_list, sizeof(DBenum) * this->attr_num);
	this->tuple_size = tupleLen(this->attr_type, this->attr_num);
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

// new table create from a base table through `select` 
Table::Table(const std::string & table_name, 
    const Table * based_table)
    :table_flag(DB_TEMPORAL_TABLE),
    table_name(table_name),
    is_primary_key(false){
    this->attr_num = based_table->attr_num;
    this->attr_type = new DBenum[this->attr_num];
    memcpy(this->attr_type, based_table->attr_type, sizeof(DBenum) * this->attr_num);
	this->tuple_size = tupleLen(this->attr_type, this->attr_num);
	this->attr_name = new string[this->attr_num];
    this->key_index = based_table->key_index;
    for(int i = 0; i < this->attr_num; i++){
        this->attr_name[i] = based_table->attr_name[i];
    }
    this->indirect_attr_map = based_table->indirect_attr_map;
}

// new table create from a base table through `project` 
Table::Table(const std::string & table_name, 
    const Table * based_table, 
    const AttributesAliasVector & attr_name_alias)
    :table_flag(DB_TEMPORAL_TABLE),
    table_name(table_name),
    is_primary_key(false){
    this->attr_num = (int)attr_name_alias.size();
    this->attr_type = new DBenum[this->attr_num];
    this->attr_name = new string[this->attr_num];
    for(unsigned int i = 0; i < attr_name_alias.size(); i++){
        this->attr_name[i] = attr_name_alias[i].AttrName;
        this->attr_type[i] = based_table->attr_type[attr_name_alias[i].OriginIndex];
        
        IndirectAttrMap::const_iterator iter = based_table->indirect_attr_map.find(this->attr_name[i]);
        if(iter != based_table->indirect_attr_map.end()){
            this->indirect_attr_map.insert(*iter);
        }
        
        if(attr_name_alias[i].OriginIndex == based_table->getKeyIndex()){
            this->key_index = i;
        }
    }
	this->tuple_size = tupleLen(this->attr_type, this->attr_num);
}

// new table created from two base table, through `natural join` 
Table::Table(const std::string & table_name, 
    const Table * based_table1, 
    const Table * based_table2, 
    const AttributesAliasVector & attr_name_alias)
    :table_flag(DB_TEMPORAL_TABLE),
    table_name(table_name){
    this->attr_num = attr_name_alias.size();
    this->attr_type = new DBenum[this->attr_num];
    this->attr_name = new string[this->attr_num];
    for(unsigned int i = 0; i < attr_name_alias.size(); i++){
        this->attr_name[i] = attr_name_alias[i].AttrName;            
        
        int origin_index = attr_name_alias[i].OriginIndex;
        if(origin_index < 0){
            origin_index = ~origin_index;
            this->attr_type[i] = based_table2->attr_type[origin_index];
			// move indirect attr map from old table to new table
			IndirectAttrMap::const_iterator iter = based_table2->indirect_attr_map.find(this->attr_name[i]);
			if (iter != based_table2->indirect_attr_map.end()) {
				this->indirect_attr_map.insert(*iter);
			}
        }
        else{
            this->attr_type[i] = based_table1->attr_type[origin_index];
			// move indirect attr map from old table to new table			
			IndirectAttrMap::const_iterator iter = based_table1->indirect_attr_map.find(this->attr_name[i]);
			if (iter != based_table1->indirect_attr_map.end()) {
				this->indirect_attr_map.insert(*iter);
			}
			// set new key
			if(origin_index == based_table1->getKeyIndex()){
                this->key_index = origin_index;
            }
        }
    }
	this->tuple_size = tupleLen(this->attr_type, this->attr_num);
}

// new table created from two base table, through `natural join` 
Table::Table(const std::string & table_name, 
    const Table * based_table1, 
    const Table * based_table2, 
    const AttributesAliasVector & attr_name_alias,
    const IndirectAttrMap & indirect_attr_map)
    :table_flag(DB_TEMPORAL_TABLE),
    table_name(table_name),
    indirect_attr_map(indirect_attr_map){
    this->attr_num = attr_name_alias.size();
    this->attr_type = new DBenum[this->attr_num];
    this->attr_name = new string[this->attr_num];
	for (unsigned int i = 0; i < attr_name_alias.size(); i++) {
		this->attr_name[i] = attr_name_alias[i].AttrName;

		int origin_index = attr_name_alias[i].OriginIndex;
		if (origin_index < 0) {
			origin_index = ~origin_index;
			this->attr_type[i] = based_table2->attr_type[origin_index];
			// move indirect attr map from old table to new table
			IndirectAttrMap::const_iterator iter = based_table2->indirect_attr_map.find(this->attr_name[i]);
			if (iter != based_table2->indirect_attr_map.end()) {
				this->indirect_attr_map.insert(*iter);
			}
		}
		else {
			this->attr_type[i] = based_table1->attr_type[origin_index];
			// move indirect attr map from old table to new table			
			IndirectAttrMap::const_iterator iter = based_table1->indirect_attr_map.find(this->attr_name[i]);
			if (iter != based_table1->indirect_attr_map.end()) {
				this->indirect_attr_map.insert(*iter);
			}
			// set new key
			if (origin_index == based_table1->getKeyIndex()) {
				this->key_index = origin_index;
			}
		}
    }
}

Table::~Table(){
    if(this->table_flag == DB_MATERIALIZED_TABLE){
        catalog.DropTable(this->table_name);
    }
    delete [] this->attr_name;
    delete [] this->attr_type;
}


pair<TableIterator*, TableIterator*> Table::PrimaryIndexFilter(const string & op, const string & value){
    DBenum key_type = this->attr_type[this->key_index];
    uint8_t* value_ptr = new uint8_t[typeLen(key_type)];
    string2Bytes(value, key_type, value_ptr);
    if(this->table_flag == DB_TEMPORAL_TABLE){
        TupleKey map_key = TupleKey((const void*)value_ptr, key_type);
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
            return make_pair((TableIterator *)iterator_begin, (TableIterator *)iterator_end);
        }
        else{ //op == "!="
            return make_pair(this->begin(), this->end());
        }
    }
    else{
        uint32_t begin_addr, end_addr;
        Table::BlockFilter(op, (const void*)value_ptr, &begin_addr, &end_addr);
        MaterializedTable_Iterator* iterator_begin 
            = new MaterializedTable_Iterator(begin_addr, 0, this->attr_num, this->attr_type, this->key_index);
        MaterializedTable_Iterator* iterator_end 
            = new MaterializedTable_Iterator(end_addr, 0, this->attr_num, this->attr_type, this->key_index);
        return make_pair((TableIterator *)iterator_begin, (TableIterator *)iterator_end);
    }
    delete [] value_ptr;
}

void Table::materialized_InsertTuple(const void ** tuple_data_ptr){
    uint32_t record_block_addr;
    /* do finding the tuple key through index */
    DBenum key_type = this->attr_type[this->key_index];
    uint32_t index_root = this->index_addr;
    AutoPtr<SearchResult> result_ptr(index_manager.searchEntry(DB_BPTREE_INDEX, index_root,
                         key_type, tuple_data_ptr[this->key_index]));
    if (result_ptr.raw_ptr) {
        if (compare(result_ptr->node->getKey(result_ptr->index),
			tuple_data_ptr[this->key_index], key_type) == 0) {
            if (this->is_primary_key) {
				throw DuplicatedPrimaryKey();
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
        int i = record_block_ptr->FindTupleIndex(tuple_data_ptr[this->key_index]);
        // if i == -1, then this block is empty.
        if (i >= 0 && i < record_block_ptr->RecordNum() &&
            compare(tuple_data_ptr[this->key_index], record_block_ptr->GetDataPtr(i, this->key_index), key_type) == 0) {
			throw DuplicatedPrimaryKey();
        }
        record_block_ptr->InsertTupleByIndex(tuple_data_ptr, i >= 0 ? i : 0);
    }
    else {
        record_block_ptr->InsertTuple(tuple_data_ptr);
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

void Table::temporal_InsertTuple(const void ** tuple_data_ptr){
	Tuple tuple(this->attr_num, this->attr_type);
	for (int i = 0; i < this->attr_num; i++) {
		memcpy(tuple[i], tuple_data_ptr[i], typeLen(this->attr_type[i]));
	}
    TupleKey new_tuple_key = TupleKey(tuple[this->key_index], this->attr_type[this->key_index]);
    this->table_data.insert(make_pair(new_tuple_key, tuple));
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

void Table::materialize(){
    if(this->table_flag != DB_TEMPORAL_TABLE){
        return;
    }
    catalog.CreateTable(this->table_name, this->attr_name, this->attr_type, this->attr_num, this->key_index);
    TableMeta* table_meta = catalog.GetTableMeta(this->table_name);
    this->index_addr = table_meta->primary_index_addr;
    this->data_addr = table_meta->table_addr;
    delete table_meta;

	TemporalTableDataMap::iterator iter;
    for(iter = this->table_data.begin(); iter != this->table_data.end(); iter++){
        this->materialized_InsertTuple(iter->second.entry_ptr());
    }
    this->table_data.clear();
    this->table_flag = DB_MATERIALIZED_TABLE;
}  