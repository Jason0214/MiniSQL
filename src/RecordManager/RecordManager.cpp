#include "RecordManager.h"
#include "../BufferManager/BufferManager.h"
#include "../CatalogManager/Catalog.h"

using namespace std;

static BufferManager & buffer_manager = BufferManager::Instance();
static Catalog & catalog = Catalog::Instance();

void RecordManager::doubleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
				const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
	uint32_t src_table1_next_block_addr = src_table1->getDataBlockAddr();
	uint32_t src_table2_next_block_addr = src_table2->getDataBlockAddr();
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];
	while(src_table1_next_block_addr != 0){
		RecordBlock* src_table1_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table1_next_block_addr));
		src_table1_block->Format(src_table1->getAttrTypeList(), src_table1->getAttrNum(), src_table1->getKeyIndex());
		src_table1_next_block_addr = src_table1_block->NextBlockIndex();
		while(src_table2_next_block_addr != 0){
			RecordBlock* src_table2_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table2_next_block_addr));
			src_table2_block->Format(src_table2->getAttrTypeList(), src_table2->getAttrNum(), src_table2->getKeyIndex());
			src_table2_next_block_addr = src_table2_block->NextBlockIndex();
			for(int i = 0; i < src_table1_block->RecordNum(); i++){
				for(int j = 0; j < src_table2_block->RecordNum(); j++){
					// check common index
					bool match = true;
					for(unsigned int k = 0; k < commonAttrIndex.size(); k++){
						int index_1 = commonAttrIndex[k].first;
						int index_2 = commonAttrIndex[k].second;
						if(compare(src_table1_block->GetDataPtr(i, index_1), src_table2_block->GetDataPtr(j, index_2), 
							src_table1->getAttrType(index_1)) != 0){
							match = false;
							break;
						}
					}
					if(match){
						for(unsigned int k = 0; k < attr_alias.size(); k++){
							int origin_index = attr_alias[k].OriginIndex;
							if(origin_index < 0){
								origin_index = ~origin_index;
								tuple_data_list[k] = src_table2_block->GetDataPtr(j, origin_index);
							}
							else{
								tuple_data_list[k] = src_table1_block->GetDataPtr(i, origin_index);
							}
						}
                        dst_table->insertTuple(tuple_data_list);
					}
				}
			}
			buffer_manager.ReleaseBlock(src_table2_block);
		}
		buffer_manager.ReleaseBlock(src_table1_block);
	}
	delete [] tuple_data_list;
}

// src_table2 is Temporal_table
void RecordManager::singleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
						const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
	uint32_t src_table1_next_block_addr = src_table1->getDataBlockAddr();
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];	
	TableIterator* src_table2_begin = src_table2->begin();
	TableIterator* src_table2_end = src_table2->end();
	while(src_table1_next_block_addr != 0){
		RecordBlock* src_table1_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table1_next_block_addr));
		src_table1_block->Format(src_table1->getAttrTypeList(), src_table1->getAttrNum(), src_table1->getKeyIndex());
		src_table1_next_block_addr = src_table1_block->NextBlockIndex();
		for(int i = 0; i < src_table1_block->RecordNum(); i++){
			for(TableIterator* iter = src_table2_begin; !iter->isEqual(src_table2_end); iter->next()){
				bool match = true;
				for(unsigned int k = 0; k < commonAttrIndex.size(); k++){
					int index_1 = commonAttrIndex[k].first;
					int index_2 = commonAttrIndex[k].second;
					if(compare(src_table1_block->GetDataPtr(i, index_1), iter->getAttrData(index_2), 
						src_table1->getAttrType(index_1)) != 0){
						match = false;
						break;
					}
				}
				if(match){
					for(unsigned int k = 0; k < attr_alias.size(); k++){
						int origin_index = attr_alias[k].OriginIndex;
						if(origin_index < 0){
							origin_index = ~origin_index;
							tuple_data_list[k] = iter->getAttrData(origin_index);
						}
						else{
							tuple_data_list[k] = src_table1_block->GetDataPtr(i, origin_index);
						}
					}
                    dst_table->insertTuple(tuple_data_list);
				}
			}		
		}
	}
	delete src_table2_begin;
	delete src_table2_end;
	delete [] tuple_data_list;
}

void RecordManager::tupleNestedLoopNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
						const AttributesAliasVector & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];
	TableIterator* src_table1_begin = src_table1->begin();
	TableIterator* src_table1_end = src_table1->end();
	TableIterator* src_table2_begin = src_table2->begin();
	TableIterator* src_table2_end = src_table2->end();
	for(TableIterator* iter1 = src_table1_begin; !iter1->isEqual(src_table1_end); iter1->next()){
		for(TableIterator* iter2 = src_table2_begin; !iter2->isEqual(src_table2_end); iter1->next()){
			bool match = true;
			for(unsigned int k = 0; k < commonAttrIndex.size(); k++){
				int index_1 = commonAttrIndex[k].first;
				int index_2 = commonAttrIndex[k].second;
				if(compare(iter1->getAttrData(index_1), iter2->getAttrData(index_2), 
					src_table1->getAttrType(index_1)) != 0){
					match = false;
					break;
				}					
			}
			if(match){
				for(unsigned int k = 0; k < attr_alias.size(); k++){
					int origin_index = attr_alias[k].OriginIndex;
					if(origin_index < 0){
						origin_index = ~origin_index;
						tuple_data_list[k] = iter2->getAttrData(origin_index);
					}
					else{
						tuple_data_list[k] = iter1->getAttrData(origin_index);
					}
				}
                dst_table->insertTuple(tuple_data_list);
			}
		}
	}
	delete src_table1_begin;
	delete src_table1_end;
	delete src_table2_begin;
	delete src_table2_end;
	delete [] tuple_data_list;
}



void RecordManager::doubleBlockNestedJoin(Table* src_table1, Table* src_table2, Table* dst_table,
						const AttributesAliasVector & attr_alias){
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];
	uint32_t src_table1_next_block_addr = src_table1->getDataBlockAddr();
	uint32_t src_table2_next_block_addr = src_table2->getDataBlockAddr();
	while(src_table1_next_block_addr != 0){
		RecordBlock* src_table1_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table1_next_block_addr));
		src_table1_block->Format(src_table1->getAttrTypeList(), src_table1->getAttrNum(), src_table1->getKeyIndex());
		src_table1_next_block_addr = src_table1_block->NextBlockIndex();
		while(src_table2_next_block_addr != 0){
			RecordBlock* src_table2_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table2_next_block_addr));
			src_table2_block->Format(src_table2->getAttrTypeList(), src_table2->getAttrNum(), src_table2->getKeyIndex());
			src_table2_next_block_addr = src_table2_block->NextBlockIndex();
			for(int i = 0; i < src_table1_block->RecordNum(); i++){
				for(int j = 0; j < src_table2_block->RecordNum(); j++){
					for(unsigned int k = 0; k < attr_alias.size(); k++){
						int origin_index = attr_alias[k].OriginIndex;
						if(origin_index < 0){
							origin_index = ~origin_index;
							tuple_data_list[k] = src_table2_block->GetDataPtr(j, origin_index);
						}
						else{
							tuple_data_list[k] = src_table1_block->GetDataPtr(i, origin_index);
						}
					}
                    dst_table->insertTuple(tuple_data_list);
				}
			}
			buffer_manager.ReleaseBlock(src_table2_block);
		}
		buffer_manager.ReleaseBlock(src_table1_block);
	}	

	delete [] tuple_data_list;
}

void RecordManager::singleBlockNestedJoin(Table* src_table1, Table* src_table2, Table* dst_table,
						const AttributesAliasVector & attr_alias){
	uint32_t src_table1_next_block_addr = src_table1->getDataBlockAddr();
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];	
	TableIterator* src_table2_begin = src_table2->begin();
	TableIterator* src_table2_end = src_table2->end();
	while(src_table1_next_block_addr != 0){
		RecordBlock* src_table1_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src_table1_next_block_addr));
		src_table1_block->Format(src_table1->getAttrTypeList(), src_table1->getAttrNum(), src_table1->getKeyIndex());
		src_table1_next_block_addr = src_table1_block->NextBlockIndex();
		for(int i = 0; i < src_table1_block->RecordNum(); i++){
			for(TableIterator* iter = src_table2_begin; !iter->isEqual(src_table2_end); iter->next()){
				for(unsigned int k = 0; k < attr_alias.size(); k++){
					int origin_index = attr_alias[k].OriginIndex;
					if(origin_index < 0){
						origin_index = ~origin_index;
						tuple_data_list[k] = iter->getAttrData(origin_index);
					}
					else{
						tuple_data_list[k] = src_table1_block->GetDataPtr(i, origin_index);
					}
				}
                dst_table->insertTuple(tuple_data_list);
			}		
		}
	}
	delete src_table2_begin;
	delete src_table2_end;
	delete [] tuple_data_list;
}

void RecordManager::tupleNestedLoopJoin(Table* src_table1, Table* src_table2, Table* dst_table,
						const AttributesAliasVector & attr_alias){
	const void** tuple_data_list = new const void*[dst_table->getAttrNum()];
	TableIterator* src_table1_begin = src_table1->begin();
	TableIterator* src_table1_end = src_table1->end();
	TableIterator* src_table2_begin = src_table2->begin();
	TableIterator* src_table2_end = src_table2->end();
	for(TableIterator* iter1 = src_table1_begin; !iter1->isEqual(src_table1_end); iter1->next()){
		for(TableIterator* iter2 = src_table2_begin; !iter2->isEqual(src_table2_end); iter1->next()){
			for(unsigned int k = 0; k < attr_alias.size(); k++){
				int origin_index = attr_alias[k].OriginIndex;
				if(origin_index < 0){
					origin_index = ~origin_index;
					tuple_data_list[k] = iter2->getAttrData(origin_index);
				}
				else{
					tuple_data_list[k] = iter1->getAttrData(origin_index);
				}
			}
            dst_table->insertTuple(tuple_data_list);
		}
	}
	delete src_table1_begin;
	delete src_table1_end;
	delete src_table2_begin;
	delete src_table2_end;
	delete [] tuple_data_list;
}