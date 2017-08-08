#pragma once
#include <string>
#include <iostream>

using namespace std;

class Exception:public std::exception{
public:
	Exception(const string & msg)
	{
		Message = msg;
	}
	string Message;
};

class DuplicatedTableName :public Exception {
public:
	DuplicatedTableName(const string & table_name) 
		:Exception("Table `" + table_name + "` already exist.") {}
};

class AttrNumberUnmatch : public Exception{
public:
	AttrNumberUnmatch() : Exception("Attributs number didn't much with table schema."){}
};

class AttrTypeUnmatch : public Exception{
public:
	AttrTypeUnmatch(const string & value) 
		: Exception("Attribute with value`" + value + "` has the wrong type."){}
};

class TableNotFound : public Exception {
public:
	TableNotFound(string msg) :Exception(msg){}
};

class IndexNotFound: public Exception{
public:
	IndexNotFound(std::string index_name) 
		:Exception("Secondary index named `" + index_name + "` not found."){}
};

class DuplicatedPrimaryKey : public Exception {
public:
	DuplicatedPrimaryKey():Exception("Didn't satisfy primary key." ) {}
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

class SameAttrNameWithDifferType : public Exception{
public:
	SameAttrNameWithDifferType(string msg) : Exception(msg){}
};