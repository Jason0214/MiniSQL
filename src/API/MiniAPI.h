// The API module is implemented using low level modules including 
// IndexManager,RecordManager,and CatalogManager. It can be used by
// high level modules like Interpretor to carry out basic SQL operations
#pragma once
#include <string>
#include <iostream>

//select data from table, put new table in result
void select(std::string table, std::string* operands, std::string* operators, int cnt, std::string result);