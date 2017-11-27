#ifndef __TABLE__
#define __TABLE__

#include <map>
#include <sstream>
#include "RecordStructures.h"
#include "TableIterator.h"

#define MAX_TABLE_SIZE 30 * (1 << 20) //30MB

class Table{
public:
    Table(const std::string & table_name);

    Table(const std::string & table_name, 
        const Table* based_table);

    Table(const std::string & table_name, 
        const Table* based_table, 
        const AttributesAliasVector & attr_name_alias);

    Table(const std::string & table_name, 
        const Table* based_table1,
        const Table* based_table2,
        const AttributesAliasVector & attr_name_alias);

    Table(const std::string & table_name, 
        const Table * based_table1, 
        const Table * based_table2, 
        const AttributesAliasVector & attr_name_alias,
        const IndirectAttrMap & indirect_attr_map);

    ~Table();

    TableIterator* begin(){
        if(this->table_flag == DB_TEMPORAL_TABLE){
            return (TableIterator*)new TemporalTable_Iterator(this->table_data.begin());
        }
        else{
            MaterializedTable_Iterator* ret = new MaterializedTable_Iterator(this->data_addr,
                                                                         0,
                                                                         this->attr_num,
                                                                         this->attr_type,
                                                                         this->key_index);
            if(ret->block_ptr->RecordNum() == 0){
                delete ret;
                return this->end();
            }
            return (TableIterator*)ret;
        }
    }

    TableIterator* end(){
        if(this->table_flag == DB_TEMPORAL_TABLE){
            return (TableIterator*)new TemporalTable_Iterator(this->table_data.end());
        }
        else{
            return (TableIterator*)new MaterializedTable_Iterator(0, 0, this->attr_num,this->attr_type,this->key_index);
        }
    }

    // use primary index to reduce the search area
    // when doing tuple comparision 
    std::pair<TableIterator*, TableIterator*> PrimaryIndexFilter(const std::string & op, const std::string & value);

    void insertTuple(const void** tuple_data_ptr){
        if(this->table_flag == DB_TEMPORAL_TABLE){
            this->temporal_InsertTuple(tuple_data_ptr);
            //check table size, approximately
            size_t total_size = this->tuple_size * this->table_data.size();
            if(total_size >= MAX_TABLE_SIZE){
                this->materialize();
            }
        }
        else{
            //DB_MATERIALIZED_TABLE, DB_ONDISC_TABLE
            this->materialized_InsertTuple(tuple_data_ptr);
        }
    }

    // for materialized table, use primary index to reduce the 
    // number of blocks need to be searched.
    void BlockFilter(const std::string & op,
                    const void* value,
                    uint32_t* begin_block,
                    uint32_t* end_block);

    void insertIndirectAttr(const std::string & attr_alias, const std::string & table_name, const std::string & attr_name){
        this->indirect_attr_map.insert(make_pair(attr_alias, make_pair(table_name, attr_name)));
    }

    // only used in output table meta
    const std::string getFullAttrName(int index) const{
        const std::string & attr_name = this->getAttrName(index);
        IndirectAttrMap::const_iterator result = this->indirect_attr_map.find(attr_name);
        if(result == this->indirect_attr_map.end()){
            return attr_name;
        }
        else{
            return result->second.first + "." + result->second.second;
        }
    }

    const std::string & getTableName() const{
        return this->table_name;
    }
    const std::string & getAttrName(int index) const{
        return this->attr_name[index];
    }
    const std::string & getKeyAttrName() const{
        return this->attr_name[this->key_index];
    }
    const DBenum* getAttrTypeList() const{
        return this->attr_type;
    }
    DBenum getAttrType(int index) const{
        return this->attr_type[index];
    }
    DBenum flag() const{
        return this->table_flag;
    }
    uint32_t getDataBlockAddr() const{
        return this->data_addr;
    }
    void updateDataBlockAddr(uint32_t new_addr){
        this->data_addr = new_addr;
    }
    uint32_t getIndexRoot() const{
        return this->index_addr;
    }
    void updateIndexRoot(uint32_t new_root){
        this->index_addr = new_root;
    }
    int getAttrNum() const{
        return this->attr_num;
    }
    int getKeyIndex() const{
        return this->key_index;
    }
    bool isPrimaryKey() const{
        return this->is_primary_key;
    }
private:
    Table(const Table &);    
    const Table & operator=(const Table &);

    void materialized_InsertTuple(const void ** );
    void temporal_InsertTuple(const void **);
    void materialize();

    DBenum table_flag;

    std::string table_name;
    DBenum* attr_type;
	size_t tuple_size;
    std::string* attr_name;
    int attr_num;
    int key_index;
    bool is_primary_key;

    // map for attributes in the format of `tableName`.`AttrName`
    IndirectAttrMap indirect_attr_map;
    
    // list is only used for DB_TEMPORAL table
	TemporalTableDataMap table_data;

    // only used for table on disc
    uint32_t index_addr;
    uint32_t data_addr;
};



#endif