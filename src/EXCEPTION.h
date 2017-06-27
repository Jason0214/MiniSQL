#pragma once
#include <string>
#include <iostream>

class Exception:public std::exception{

};

class DuplicatedTableName :public Exception {
public:
	DuplicatedTableName(const char*){}
};

class TableNotFound : public Exception {
public:
	TableNotFound(const char*){}
};

class IndexNotFound: public Exception{
public:
	IndexNotFound(std::string table_name="", int key_index=-1):table_name(table_name),key_index(key_index){}
	std::string table_name;
	int key_index;
};

class DuplicatedIndexName : public Exception{
};

class DuplicatedIndex: public Exception{
public:
	DuplicatedIndex(const char*, int key){}
};

class AttributeNotFound : public Exception {

};

class DiscFailure : public Exception {

};

class DatabaseNotFound : public Exception {

};

class DatabaseNotSelected : public Exception {

};
