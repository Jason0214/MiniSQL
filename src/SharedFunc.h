#ifndef _SHARED_FUNC_
#define _SHARED_FUNC_
#include "CONSTANT.h"
#include <string>
class SmartPtr{
public:
	SmartPtr(){}
	virtual ~SmartPtr(){};
private:
	SmartPtr(const SmartPtr &);
	const SmartPtr & operator=(const SmartPtr &);
};

template<typename T>
class AutoPtr:public SmartPtr{
public:
	AutoPtr(T* t_ptr):raw_ptr(t_ptr){}
	~AutoPtr(){
		delete this->raw_ptr;
	}
	T* operator->(){
		return this->raw_ptr;
	}
	T& operator*(){
		return *this->raw_ptr;
	}
	T* raw_ptr;
};

size_t typeLen(DBenum type);

size_t tupleLen(const DBenum* type_list, int attr_num);

int compare(const void* v1, const void* v2, DBenum type);

void printByType(const void* v, DBenum type);

void string2Bytes(const std::string& value, DBenum type, void* raw_value);

#endif