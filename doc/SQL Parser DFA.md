# SQL Parser DFA

### context free grammer
1. S`-> query
2. query -> "select" attr_set "from" valid_tables
3. attr_set -> attr "," attr_set
4. attr_set -> attr
5. attr -> attrID
6. attr -> attrID "as" ID
7. attrID -> ID "." ID
8. valid_tables -> table_set "where" condition_set
9. table_set -> table
10. table_set -> table "join"/"naturaljoin"/"," table_set
11. tableID -> ID
12. tableID -> "(" query ")" 
13. table -> tableID "as" ID
14. table -> tableID
15. condition_set -> condition "and"/"or" condition_set
16. condition_set -> condition
17. condition -> attrID "="/"<="/">="/"<"/">"/"<>" attrID
18. condition -> attrID "="/"<="/">="/"<"/">"/"<>" str/int/float


### DFA






### Parsing Table

state | alias  | select | where | as | from | ID | , | . | and/or | join/njoin 
 ---- | ------ | ------ | ----- | -- | ---- |  - | - | - | ------ | ----------
1     | WAIT_SELECT | s2 |      |    |      |    |   |   |        |
2     | WAIT_ATTR_SET |  |      |    |      |s3 |   |   |        | 
3     | REDUCE_ATTR | 
4     | WAIT_ATTR_ALIAS |
5     | REDUCE_ATTR_WITH_ALIAS |
6     | REDUCE_ATTR_SET |
7     | WAIT_ATTR_SET_AGAIN