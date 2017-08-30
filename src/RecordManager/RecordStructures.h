#ifndef __RECORD_STRUCTURES__
#define __RECORD_STRUCTURES__

#include "../SharedFunc.h"
#include <vector>
#include <map>
#include <string>
#include <algorithm>

typedef std::map<std::string, std::pair<std::string, std::string> > IndirectAttrMap;

typedef struct attr_alias_struct{
	std::string AttrName;
	int OriginIndex;
}AliasStructure;

typedef std::vector<AliasStructure> AttributesAliasVector;

inline bool AttrAliasSort(const AliasStructure &v1, const AliasStructure &v2) {
	return v1.AttrName < v2.AttrName;
}

class TupleCmpInfo{
public:
    TupleCmpInfo():value(NULL),value_size(0),type(DB_TYPE_INT),origin_index(0){}
    TupleCmpInfo(const TupleCmpInfo & tuple_cmp_info){
        *this = tuple_cmp_info;
    }
    ~TupleCmpInfo(){
        delete [] this->value;
    }
    void init(int index, DBenum type, const std::string & value_in_str){
        this->type = type;
        this->origin_index = index;
        this->value_size = typeLen(type);
        this->value = new uint8_t[this->value_size];
        string2Bytes(value_in_str, type, this->value);
    }
    TupleCmpInfo & operator=(const TupleCmpInfo & tuple_cmp_info){
        if(this != &tuple_cmp_info){
            this->value = tuple_cmp_info.value;
            this->value_size = tuple_cmp_info.value_size;
            this->type = tuple_cmp_info.type;
            this->value = new uint8_t[this->value_size];
            memcpy(this->value, tuple_cmp_info.value, this->value_size);
        }
        return *this;
    }

    uint8_t* value;
    int value_size;
    DBenum type;
    int origin_index;
};

typedef std::vector<TupleCmpInfo> TupleComparisonVector;

class Tuple{
public:
    Tuple(int attr_num, const DBenum* attr_type_list);
    Tuple(const Tuple & t){
        *this = t;
    }
    ~Tuple(){
        delete [] this->tuple_data;
        delete [] this->entry;
    }

    void* data_ptr(){
        return (void*)(this->tuple_data);
    }

    const void ** entry_ptr() const{
        return (const void**)(this->entry);
    }

    void* operator[](int index){
        return (void*)(this->entry[index]);
    }
	const void* operator[](int index)const{
		return (const void*)(this->entry[index]);
	}

    Tuple & operator=(const Tuple & rightv);
private:
    uint8_t* tuple_data;
    int tuple_size;
    uint8_t** entry;
    int attr_num;
};


// used as the key of std::map
class TupleKey{
public:
    TupleKey(const void* key_data, DBenum key_type);
    TupleKey(const TupleKey & t){
        *this = t;
    }
    ~TupleKey(){
        delete [] this->key_data;
    }
	friend bool operator<(const TupleKey & leftv, const TupleKey & rightv);
    TupleKey & operator=(const TupleKey & t);
    uint8_t* key_data; 
    DBenum type;
};

typedef std::map<TupleKey, Tuple> TemporalTableDataMap;

#endif