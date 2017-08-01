#ifndef __RECORD_MANAGER__
#define __RECORD_MANAGER__

#include <map>
#include "Table.h"

#define MAX_TEMPORAL_TABLE_SIZE (1 << 25) //32MB

class RecordManager{
public:
	static RecordManager & Instance(){
		static RecordManager theRecordManager;
		return theRecordManager;
	}
	~RecordManager();

	void clear(){
		std::map<std::string, Table*>::iterator iter;
		for(iter = this->data.begin(); iter != this->data.end(); iter++){
			delete iter->second;
		}
	}
	void addTable(Table * table_ptr){
		this->data[table_ptr->getTableName()] = table_ptr;
	}
	Table* getTable(const std::string & table_name){
		return this->data.at(table_name);
	}

private:
	RecordManager(){};
	RecordManager(const RecordManager &);
	const RecordManager & operator=(const RecordManager &);

	std::map<std::string, Table*> data;
};


#endif