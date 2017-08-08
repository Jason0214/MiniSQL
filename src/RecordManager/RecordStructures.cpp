#include "RecordStructures.h"

Tuple::Tuple(int attr_num, const DBenum* attr_type_list):
    tuple_size(0),
    attr_num(attr_num){
    // TODO store typeLen in a temp array
    this->tuple_size = tupleLen(attr_type_list, attr_num);
    this->tuple_data = new uint8_t[this->tuple_size];
    this->entry = new uint8_t*[attr_num];
    this->entry[0] = &(this->tuple_data[0]);
    for(int i = 1; i < attr_num; i++){
        this->entry[i] = this->entry[i-1] + typeLen(attr_type_list[i-1]);
    }
}

const Tuple & Tuple::operator=(const Tuple & rightv){
	if(&rightv != this){
	    this->attr_num = rightv.attr_num;
	    this->tuple_size = rightv.tuple_size;
	    this->entry = new uint8_t*[this->attr_num];
	    this->tuple_data = new uint8_t[this->tuple_size];
	    memcpy(this->tuple_data, rightv.tuple_data, this->tuple_size);
	    this->entry[0] = &(this->tuple_data[0]);
	    for(int i = 1; i < this->attr_num; i++){
	        this->entry[i] = (uint8_t*)((const uint8_t*)rightv[i] + (this->tuple_data - rightv.tuple_data));
	    }
	}
	return *this;
}


TupleKey::TupleKey(const void* key_data, DBenum key_type):
	type(key_type)
{
    size_t len = typeLen(key_type);
    this->key_data = new uint8_t[len];
    memcpy(this->key_data, key_data, len);
}

const TupleKey & TupleKey::operator=(const TupleKey & t){
    if(this != &t){
        this->type = t.type;
        size_t len = typeLen(t.type);
        this->key_data = new uint8_t[len];
        memcpy(this->key_data, t.key_data, len);
    }
	return *this;
}

bool operator<(TupleKey const & leftv, TupleKey const & rightv)
{
	if (compare(leftv.key_data, rightv.key_data, leftv.type) < 0) {
		return true;
	}
	// FIXME:
	if (leftv.key_data < rightv.key_data) {
		return true;
	}
	return false;
}
