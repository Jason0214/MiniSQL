#pragma once
#include "BufferManager.h"
// used as smart pointer for Block* and its inheirtance,
// Block* should not be freed out side BufferManager,
// instead should call ReleaseBlock() when the stack
// got freed.
template<typename T>
class BlockPtr: public SmartPtr{
public:
	BlockPtr(T* t_ptr):raw_ptr(t_ptr){}
	~BlockPtr(){
		BufferManager::Instance().ReleaseBlock(this->raw_ptr);
	}
	T* operator->(){
		return this->raw_ptr;
	}
	T& operator*(){
		return *this->raw_ptr;
	}
	T* raw_ptr;
};
