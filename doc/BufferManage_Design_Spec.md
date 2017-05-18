# BufferManage Design Spec

``` json
Table Block Head{
    8 bits : block type (an enum value specify whether a index block or a table block etc.)
    8 bits : reserved
    32 bits : currrent block index (denotes where this block in the whole big file) 
    32 bits : next block index
    12 bits: empty ptr (store the offset of next empty byte)
    12 bits: empty len (store the length of next empty area)
}
```
