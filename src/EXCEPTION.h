#pragma once
#include <string>
#include <iostream>

using namespace std;

class Exception:public std::exception{
public:
	string Message;
	Exception(string msg)
	{
		Message = msg;
	}
};

class DuplicatedTableName :public Exception {
public:
	DuplicatedTableName(string msg) :Exception(msg) {}
};

class TableNotFound : public Exception {
public:
	TableNotFound(string msg) :Exception(msg){}
};

class IndexNotFound: public Exception{
public:
	IndexNotFound(std::string table_name = "", int key_index = -1) 
		:Exception("index not found " + table_name), table_name(table_name), key_index(key_index){}
	std::string table_name;
	int key_index;
};

class DuplicatedIndexName : public Exception{
public:
	DuplicatedIndexName(string msg) :Exception(msg){}
};

class DuplicatedIndex: public Exception{
public:
	DuplicatedIndex(string msg, int key) :Exception(msg){}
};

class AttributeNotFound : public Exception {
public:
	AttributeNotFound(string msg) :Exception(msg){}
};

class DiscFailure : public Exception {
public:
	DiscFailure(string msg) :Exception(msg){}
};

class DatabaseNotFound : public Exception {
public:
	DatabaseNotFound(string msg) :Exception(msg){}
};

class DatabaseNotSelected : public Exception {
public:
	DatabaseNotSelected(string msg) :Exception(msg){}
};

class TableAliasNotFound : public Exception {
	public:
	TableAliasNotFound(string msg) :Exception(msg) {}
};