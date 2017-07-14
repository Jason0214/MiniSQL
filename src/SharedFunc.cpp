#include "SharedFunc.h"
#include "CONSTANT.h"
#include <cstring>
#include <iostream>

using namespace std;

size_t typeLen(DBenum type) {
	switch (type) {
	case DB_TYPE_INT: return 4;
	case DB_TYPE_FLOAT: return 4;
	default: return (int)type - (int)DB_TYPE_CHAR;
	}
}

int compare(const void* v1, const void* v2, DBenum type) {
	switch (type) {
	case DB_TYPE_INT: return *(int*)v1 - *(int*)v2;
	case DB_TYPE_FLOAT: return (int)(*(float*)v1 - *(float*)v2);
	default: return strcmp((char*)v1, (char*)v2);
	}
}

void printByType(const void* v, DBenum type) {
	switch (type) {
	case DB_TYPE_INT: cout << *(int*)v << " "; break;
	case DB_TYPE_FLOAT: cout << *(float*)v << " "; break;
	default: cout << (char*)v << " "; break;
	}	
}