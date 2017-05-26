#include <iostream>
#include "BufferManager.h"

using namespace std;

int main(){
	BufferManager bf_manager;
	SchemaBlock* block_ptr= dynamic_cast<SchemaBlock*>(bf_manager.GetBlock(0));
	cout << block_ptr->BlockType() << endl;
	cout << block_ptr->BlockIndex() << endl;
	cout << block_ptr->NextBlockIndex() << endl;
	cout << block_ptr->EmptyPtr() << endl;
	cout << block_ptr->EmptyLen() << endl;
	cout << block_ptr->UserMetaAddr() << endl;
	cout << block_ptr->DBMetaAddr() << endl;
	cout << block_ptr->EmptyBlockAddr() << endl;

	Block* usr_block = bf_manager.GetBlock(1);
	cout << usr_block->BlockType() << endl;
	Block* new_block = bf_manager.CreateBlock();
	cout << new_block->BlockIndex() << endl;
	bf_manager.DeleteFromDisc(new_block);
	cout << block_ptr->EmptyBlockAddr() << endl;
	return 0;
}