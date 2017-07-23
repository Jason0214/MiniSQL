#ifndef __RECORD_STRUCTURES__
#define __RECORD_STRUCTURES__

#include "../SharedFunc.h"
#include <map>
#include <string>

typedef enum{
    TABLE_SELECT,
    TABLE_PROJECT,
    TABLE_JOIN,
    TABLE_NATURAL_JOIN,    
}TableImplement;


typedef std::map<std::string, std::string> AttrAlias;


class Tuple{
public:
    Tuple(const void** data_list, int attr_num, DBenum* attr_type_list):
    entry_num(attr_num)，
    tuple_size(0){
        for(int i = 0; i < attr_num; i++){
            this->tuple_size += typeLen(attr_type_list[i]);
        }
        this->tuple_data = new uint8_t[this->tuple_size];
        this->entry = new void*[attr_num];
        this->entry[0] = &(this->tuple_data[0])
        for(int i = 1; i < attr_num; i++){
            this->entry[i] = this->entry[i-1] + typeLen(attr_type_list[i]);
        }
    }
    Tuple(const Tuple & t){
        *this = t;
    }
    ~Tuple(){
        delete [] this->tuple_data;
        delete [] this->entry;
    }
    int getEntryNum(){
        return this->entry_num;
    }
    int getTupleSize(){
        return this->tuple_size;
    }
    void* operator[](int index){
        return (void*)(this->entry[index]);
    }

    const Tuple & operator=(const Tulpe& rightv){
        if(&rightv != this){
            this->entry_num = rightv.getEntryNum();
            this->tuple_size = rightv.getTupleSize();
            this->entry = new void*[this->entry_num];
            this->tuple_data = new uint8_t[this->tuple_size];
            memcpy(this->tuple_data, &(rightv[0]), this->tuple_size);
            this->entry[0] = &(this->tuple_data[0]);
            for(int i = 1; i < this->entry_num; i++){
                this->entry[i] = this->entry[i-1] + rightv[i] - rightv[i-1];
            }
        }
        return *this;
    }
private:
    uint8_t* tuple_data;
    int tuple_size;
    uint8_t** entry;
    int entry_num;
};


// used as the key of std::map
class TupleKey{
public:
    TupleKey(const void* key_data, DBenum key_type):
    type(key_type){
        size_t len = typeLen(key_type);
        this->key_data = new uint8_t[len];
        memcpy(this->key_data, key_data, len);
    }
    TupleKey(const TupleKey & t){
        *this = t;
    }
    ~TupleKey(){
        delete [] this->key_data;
    }
    bool operator<(TupleKey const & rightv) const
    {
        if(compare(this->key_data, rightv->key_data, this->type) < 0){
            return true;
        }
        // FIXME:
        if(this->key_data < rightv->key_data){
            return true;
        }
        return false;
    }
    const TupleKey & operator=(const TupleKey & t){
        if(this != &t){
            this->type = t.type;
            size_t len = typeLen(t.type);
            this->key_data = new uint8_t[len];
            memcpy(this->key_data, t.key_data, len);
        }
        return *this
    }

    uint8_t* key_data; 
    DBenum type;
};

#endif