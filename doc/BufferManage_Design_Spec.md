# BufferManage Design Spec

``` 
Block Head{
    8 bits : block type (an enum value specify whether a index block or a table block etc.)
    8 bits : reserved
    32 bits : currrent block index (denotes where this block in the whole big file) 
    32 bits : next block index
    16 bits: empty ptr (store the offset of next empty byte) //point to the empty address of the block
}
```


```
Schema Block (first block){
    Block head 12 Bytes,
    32 bits :  EMPTY Block Address // begin of a link list
    32 bits :  address of user priviliege table
    32 bits :  address of database table
    32 bits :  address of index table
    32 bits :  address of 
    ...
}
```

```
Buffer manager{
API:
    Block* GetBlock(int block_index)
    void DeleteFromDisc(Block*) 
    Block* CreateBlock()
Inner structure:
    a double linked list which always moving the recently used block to the head
    a hash table for fast access a block according to its block index
}
```