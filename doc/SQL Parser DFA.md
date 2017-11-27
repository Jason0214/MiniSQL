# SQL Parser DFA
     context free grammer
### Query
1. S`-> query
2. query -> "select" attr_set "from"  table_set "where" condition_set
3. query -> "select" attr_set "from" table_set
4. attr_set -> attr "," attr_set
5. attr_set -> attr
6. attr_set -> "*"
7. attr -> attrID
8. attr -> attrID "as" ID
9. attrID -> ID "." ID
10. table_set -> table
11. table_set -> table "join"/"naturaljoin"/"," table_set
12. tableID -> ID
13. tableID -> "(" query ")" 
14. table -> tableID "as" ID
15. table -> tableID
16. condition_set -> condition "and"/"or" condition_set
17. condition_set -> condition
18. condition -> attrID "="/"<="/">="/"<"/">"/"<>" attrID
19. condition -> attrID "="/"<="/">="/"<"/">"/"<>" str/int/float


### Insert
1. S`->insert
2. insert -> "insert" "into" id "values" "(" value_set ")"
3. value_set -> int/float/string "," value_set
4. value_set -> int/float/string

### Delete
1. S` -> delete
2. delete -> "delete" "from" id "where" condition_set
3. condition_set -> condition "and"/"or" condition_set
4. condition_set -> condition
5. condition -> attrID "="/"<="/">="/"<"/">"/"<>" attrID
6. condition -> attrID "="/"<="/">="/"<"/">"/"<>" str/int/float

### create table
1. S` -> create
2. create -> "create" "table" id "(" meta_set ")"
3. meta_set -> meta "," meta_set
4. meta_set -> meta
5. meta -> id type 
6. meta -> id type "primary" "key" "not" "null"
6. type -> keyword
7. type -> keyword "(" int ")"

### update
1. S` -> update
2. update -> "update" id "set" assign_set
3. assign_set -> assign "," assign_set 
4. assign_set -> assign
5. assign -> id "=" int/float/string

### Drop table
1. S` -> drop
2. drop -> "drop" "table" id

### create index
1. S` -> "create" "index" id "on" table '('column_set')'
2. column_set ->  id
3. column_set ->  id, column_set

### drop index
1. S` -> "drop" "index" id 