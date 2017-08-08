#include "SharedFunc.h"
#include "EXCEPTION.h"
#include "CONSTANT.h"
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

size_t typeLen(DBenum type) {
	switch (type) {
	case DB_TYPE_INT: return sizeof(int);
	case DB_TYPE_FLOAT: return sizeof(float);
	default: return (int)type - (int)DB_TYPE_CHAR + 1;
	}
}

size_t tupleLen(const DBenum* attr_type_list, int attr_num){
	size_t ret = 0;
	for(int i = 0; i < attr_num; i++){
		ret += typeLen(attr_type_list[i]);
	}
	return ret;
}

int compare(const void* v1, const void* v2, DBenum type) {
	switch (type) {
	case DB_TYPE_INT: return *(int*)v1 - *(int*)v2;
	case DB_TYPE_FLOAT: return (int)(*(float*)v1 - *(float*)v2);
	default: return strcmp((char*)v1, (char*)v2);
	}
}

void string2Bytes(const std::string& value, DBenum type, void* raw_value){
	// convert `value` to const void*
	stringstream ss(value);
	ss.exceptions(std::ios::failbit);
	switch (type) {
	case DB_TYPE_INT:
		ss >> *(int*)raw_value;
		break;
	case DB_TYPE_FLOAT:
		ss >> *(float*)raw_value;
		break;
	default:
		if (type - DB_TYPE_CHAR < (int)value.length()) {
			throw AttrTypeUnmatch(value);
		}
		memcpy(raw_value, ss.str().c_str(), typeLen(type));
		break;
	}
}

void printByType(const void* v, DBenum type) {
	switch (type) {
	case DB_TYPE_INT: cout << *(int*)v << " "; break;
	case DB_TYPE_FLOAT: cout << *(float*)v << " "; break;
	default: cout << (char*)v << " "; break;
	}	
}