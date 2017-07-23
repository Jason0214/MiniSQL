#ifndef __TABLE__
#define __TABLE__

#include <map>
#include "RecordStructures.h"
#include "TableInterator.h"

class Table{
public:
    Table(const std::string & table_name);

    Table(const std::string & table_name, 
        const Table & based_table, 
        const AttrAlias & attr_name_map, 
        TableImplement op);

    Table(const std::string & table_name, 
        const Table & based_table1, 
        const AttrAlias & attr_name_map1, 
        const Table & based_table2, 
        const AttrAlias & attr_name_map2,
        TableImplement op);

    TableIterator* begin(){
        if(this->table_flag = DB_TEMPORAL){
            return new TemporalTable_iterator(this->table_data.begin());
        }
        else{
            return new MaterializedTable_iterator(this->data_addr,
                                                    0,
                                                    this->attr_num,
                                                    this->attr_type,
                                                    this->key_index);
        }
    }

    TableIterator* end(){
        if(this->table_flag == DB_TEMPORAL){
            return new TemporalTable_iterator(this->table_data.end());
        }
        else{
            return new MaterializedTable_iterator(0,0,this->attr_num,this->attr_type,this->key_index);
        }
    }

    pair<TableIterator*, TableIterator*> PrimaryFilter(char op, const void* value);

    void insertTule(const void** tuple_data){
        if(this->table_flag == DB_TEMPORAL){
            this->temporal_InsertTuple(tuple_data);
        }
        else{
            //DB_MATERIALIZED
            this->materialized_InsertTuple(tuple_data);
        }
        //TODO: check table size
    }

    ~Table(){
        delete [] attr_name;
        delete [] attr_type;
    }
    const std::string & getAttrName(int index) const{
        return this->attr_name[index];
    }
    const std::string & getKeyAttrName() const{
        return this->attr_name[this->key_index];
    }
    DBenum getAttrType(int index) const{
        return this->attr_type[index];
    }
private:
    Table(const Table &);    
    const Table & operator=(const Table &);

    void materialized_InsertTuple(const void** );
    void temporal_InsertTuple(const void** );

    DBenum table_flag;

    std::string table_name;
    DBenum* attr_type;
    std::string* attr_name;
    int attr_num;
    int key_index;
    bool is_primary_key;
    
    // list is only used for DB_TEMPORAL table
    std::map<TupleKey, Tuple> table_data;

    // only used for DB_MATERIALIZED table
    uint32_t index_addr;
    uint32_t data_addr;
};



#endif