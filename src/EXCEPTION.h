#pragma once
#include <cstring>
#include <string>

class Exception:public std::exception{
public:
	Exception(const std::string & msg)
	{
		memcpy(err, msg.c_str(), 255*sizeof(char));
		err[511] = 0;
	}
	char err[512];
};

class ParseError: public Exception{
public:
    ParseError(const std::string & content, const std::string & info)
            :Exception(std::string("Parser Error: ")+"at '" + content + "', " + info){}
};

class ExecuteError: public Exception{
public:
    ExecuteError(const std::string & content, const std::string & info)
            :Exception("Execute Error: '"+content + "' " + info){}
};

class LexingError: public Exception{
public:
    LexingError(const std::string & info)
            :Exception("Lexer Error: "+info){}
};

class TODO: public Exception{
public:
    TODO(const std::string & info)
            :Exception("TODO: "+info){}
};

class FalseCondition: public Exception{
public:
    FalseCondition():Exception(""){}
};

class DuplicatedTableName :public Exception {
public:
	DuplicatedTableName(const std::string & table_name) 
		:Exception("Table `" + table_name + "` already exist") {}
};

class AttrNumberUnmatch : public Exception{
public:
	AttrNumberUnmatch(): Exception("Attributs number does not much with table schema"){}
};

class AttrTypeUnmatch : public Exception{
public:
	AttrTypeUnmatch(const std::string & value) 
		: Exception("Attribute with value`" + value + "` has the wrong type"){}
};

class TableNotFound : public Exception {
public:
	TableNotFound(const std::string & table) :Exception("Table `"+table+"` not found"){}
};

class IndexNotFound: public Exception{
public:
	IndexNotFound(const std::string & index_name) 
		:Exception("Secondary index named `" + index_name + "` not found."){}
};

class DuplicatedPrimaryKey : public Exception {
public:
	DuplicatedPrimaryKey()
	:Exception("Updated record against primary key constrain") {}
};

class DuplicatedIndexName : public Exception{
public:
	DuplicatedIndexName(const std::string & index_name) 
	:Exception("index named `"+index_name+"` already exist"){}
};

class DuplicatedIndex: public Exception{
public:
	DuplicatedIndex(const std::string & table_name, const std::string & attr) 
	:Exception("Secondary index on `"+table_name+"`("+attr+") already exist"){}
};

class AttributeNotFound : public Exception {
public:
	AttributeNotFound(const std::string & attr) 
	:Exception("Attribute named `"+attr+"` not found"){}
};

class DiscFailure : public Exception {
public:
	DiscFailure(const std::string & msg) 
	:Exception("Disk failure"){}
};

class DatabaseNotFound : public Exception {
public:
	DatabaseNotFound(const std::string & db_name) 
	:Exception("Database named `" + db_name + "` not found"){}
};

class DatabaseNotSelected : public Exception {
public:
	DatabaseNotSelected(const std::string & msg)
	 :Exception("Database not selected"){}
};

class TableAliasNotFound : public Exception {
public:
	TableAliasNotFound(const std::string & alias) 
	:Exception("Table alias `"+alias+"` not found") {}
};

class SameAttrNameWithDifferType : public Exception{
public:
	SameAttrNameWithDifferType(const std::string & msg) 
	: Exception("Inexplicit attribute name in join"){}
};
