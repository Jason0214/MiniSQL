# BufferManage Design Spec

``` 
Block{
    8 bits : block type (an enum value specify whether a index block or a table block etc.)
    8 bits : reserved
    32 bits : currrent block index (denotes where this block in the whole big file) 
    32 bits : next block index
    ...
}
```


```
Schema Block (first block){
    Block head 10 Bytes,
    16 bits: empty ptr (store the offset of next empty byte) //point to the empty address of the block
    32 bits :  EMPTY Block Address // begin of a link list
    32 bits :  address of user priviliege table
    32 bits :  address of database table
    32 bits :  address of index table
    32 bits :  address of index/view etc.
    ...
}
```

```
Buffer manager{
Interface:
    Block* GetBlock(uint32 block_index)
    void ReleaseBlock(uint32 block_index)
    void DeleteBlock(uint32 block_index) 
    uint32 CreateBlock()
Inner structure:
    a double linked list which always moving the recently used block to the head
    a hash table for fast access a block according to its block index
}
```

``` cpp
/* invoke buffer manager*/

// get the buffer manager
BufferManager & bf_manager = BufferManager::Instance();




// high level module get a block from buffer manager
Block* block_i_want = bf_manager.GetBlock(index_of_block_i_want);

    // do whatever you want

bf_manager.ReleaseBlock(index_of_block_i_want);




// create a block which is not on the disk currently
block_index_of_new_block = bf_manager.CreateBlock();
    // do the Get and Release Block




// abandon a block currently on the disk
bf_manager.DeleteBlock(index_of_block_you_want_delete)

```