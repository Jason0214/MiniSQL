# SQL Parser DFA

### context free grammer
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
