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

class DiscFailure : public Exception {

};

class DatabaseNotFound : public Exception {

};