#include "../BufferManager/BufferManager.h"

class RecordManager{
public:
	static RecordManager & Instance(){
		static RecordManager thRecordManager;
		return theRecordManager;
	}

	void InsertRecord(uint32_t table_addr, uint32_t index_addr, uint32_t key_index, DBenum* type_list, void* data_list);

	void RemoveRecord();


private:
	RecordManager();
	RecordManager(const RecordManager&);
	RecordManager & operator=(const RecordManager&);

};