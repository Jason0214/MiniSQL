#pragma once
#include <iostream>
using namespace std;

class Exception:public exception{

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
	IndexNotFound(const char*, int key){}
};

class DuplicatedIndex: public Exception{
public:
	DuplicatedIndex(const char*, int key){}
};


class DiscFailure : public Exception {

};

class DatabaseNotFound : public Exception {

};

class DatabaseNotSelected : public Exception {

};
