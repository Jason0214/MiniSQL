#pragma once
#include <cstring>
#include <string>

class Exception:public std::exception{
public:
	Exception(const std::string & msg)
	{
		memcpy(err, msg.c_str(), 255*sizeof(char));
		err[255] = 0;
	}
	char err[256];
};

class DuplicatedTableName :public Exception {
public:
	DuplicatedTableName(const std::string & table_name) 
		:Exception("Table `" + table_name + "` already exist.") {}
};

class AttrNumberUnmatch : public Exception{
public:
	AttrNumberUnmatch() : Exception("Attributs number didn't much with table schema."){}
};

class AttrTypeUnmatch : public Exception{
public:
	AttrTypeUnmatch(const std::string & value) 
		: Exception("Attribute with value`" + value + "` has the wrong type."){}
};

class TableNotFound : public Exception {
public:
	TableNotFound(const std::string & msg) :Exception(msg){}
};

class IndexNotFound: public Exception{
public:
	IndexNotFound(const std::string & index_name) 
		:Exception("Secondary index named `" + index_name + "` not found."){}
};

class DuplicatedPrimaryKey : public Exception {
public:
	DuplicatedPrimaryKey():Exception("Didn't satisfy primary key." ) {}
};

class DuplicatedIndexName : public Exception{
public:
	DuplicatedIndexName(const std::string & msg) :Exception(msg){}
};

class DuplicatedIndex: public Exception{
public:
	DuplicatedIndex(const std::string & msg, int key) :Exception(msg){}
};

class AttributeNotFound : public Exception {
public:
	AttributeNotFound(const std::string & msg) :Exception(msg){}
};

class DiscFailure : public Exception {
public:
	DiscFailure(const std::string & msg) :Exception(msg){}
};

class DatabaseNotFound : public Exception {
public:
	DatabaseNotFound(const std::string & msg) :Exception(msg){}
};

class DatabaseNotSelected : public Exception {
public:
	DatabaseNotSelected(const std::string & msg) :Exception(msg){}
};

class TableAliasNotFound : public Exception {
public:
	TableAliasNotFound(const std::string & msg) :Exception(msg) {}
};

class SameAttrNameWithDifferType : public Exception{
public:
	SameAttrNameWithDifferType(const std::string & msg) : Exception(msg){}
};

class ParseError: public Exception{
public:
	ParseError(const std::string & content, const std::string & info):Exception("at '" + content + "'' " + info){}
};