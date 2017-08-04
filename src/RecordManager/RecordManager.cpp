#include "RecordManager.h"
#include "../CatalogManager/Catalog.h"


Catalog & catalog = Catalog::Instance();
BufferManager & buffer_manager = BufferManager::Instance();

void RecordManager::doubleBlockNestedNaturalJoin(Table* src_table1, Table* src_table2, Table* dst_table,
				const AttrAlias & attr_alias,  const vector<pair<int,int> > & commonAttrIndex){
	uint32_t src1_next_block_addr = src_table1->getDataBlockAddr();
	uint32_t src2_next_block_addr = src_table2->getDataBlockAddr();
	Tuple tuple(dst_table->getAttrNum(), dst_table->getAttrTypeList());
	while(src1_next_block_addr != 0){
		RecordBlock* src1_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src1_next_block_addr));
		src1_block->Format(src_table1->getAttrTypeList(), src_table1->getAttrNum(), src_table1->getKeyIndex());
		src1_next_block_addr = src1_block->NextBlockIndex();
		while(src2_next_block_addr != 0){
			RecordBlock* src2_block = dynamic_cast<RecordBlock*>(buffer_manager.GetBlock(src2_next_block_addr));
			src2_block->Format(src_table2->getAttrTypeList(), src_table2->getAttrNum(), src_table2->getKeyIndex());
			src2_next_block_addr = src2_block->NextBlockIndex();
			for(int i = 0; i < src1_block->RecordNum(); i++){
				for(int j = 0; j < src2_block->RecordNum(); j++){
					// check common index
					bool match = true;
					for(unsigned int k = 0; k < commonAttrIndex.size(); k++){
						int index_1 = commonAttrIndex[k].first;
						int index_2 = commonAttrIndex[k].second;
						if(compare(src1_block->GetDataPtr(i, index_1), src2_block->GetDataPtr(j, index_2), 
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
								memcpy(tuple[k], src2_block->GetDataPtr(j, origin_index), src_table2->getAttrType(origin_index));
							}
							else{
								memcpy(tuple[k], src1_block->GetDataPtr(i, origin_index), src_table1->getAttrType(origin_index));
							}
							dst_table->insertTuple(tuple);
						}
					}
				}
			}
			buffer_manager.ReleaseBlock(src2_block);
		}
		buffer_manager.ReleaseBlock(src1_block);
	}
}