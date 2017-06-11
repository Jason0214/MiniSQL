# API I/O Specification

## Queries

#### -------------------------------------------------------------------

### Involved Tables and corresponding Alias:

#### Input:

Line 1: "table_info"

Line 2: (An Integer) [N] // Number of Involved Tables

Following N Lines: Each Line: [alias] [real table name] 
// separated by a space

#### No Output

#### -------------------------------------------------------------------


### Select:

#### Input:

Line 1: "select"

Line 2: [Source Table Alias]

Line 3: [Result Table Alias]

Line 4: (An Integer) [N] // Number of Comparisons (Conjunction)

Following N Lines: Each Line: [Comparand1] [Operation] [Comparand2] 

// Comparand can be one of the following: attrName, int, float, 'string'

...............................

# 算了 写不动了 找时间当面说吧 

