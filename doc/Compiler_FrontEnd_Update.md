# Compiler Tools

## Lexer

[For internal use only]

This lexer reads a string as input and generates a lexeme stream.

### Supported Syntax:

Quotes: [''], [""]

Float Number: [1.2], [.2], [2.]

Integer: [666]

Keyword: (User defined keyowrds) [select] [from]

Symbols: (User defined symbols) [;] [,] [(]

Identifier: [apple] [select1]




## Parser

[For internal use only]

Parser combinators for a general compiler, syntax undefined.

A parser will read a list of lexeme as input and generate an expression tree as output.




## SQL Lexer and Parser (For Query Only)

These two modules have been finished.

string -> [SQLLexer] -> Lexeme List -> [SQL Parser] -> Expression Tree

### Structures:

WithAlias\<T>: { T, Alias }

Attr: { AttributeName, TableName }

Table: { List<WithAlias\<string>> } // Each single table can have an alias, a table here means the natural join result of several single tables

Comparand: { TypeName, Entity } // This entity can be Attr or Int or Float or String

ComparePredict: { Operand1, Operator, Operand2 } // Operator is represented by a string, eg. "<="

ConjunctionPredict: { List\<Predict> } // A list of any kind of predicts. Note that a Between And Predict is represented by a conjunction of two simple Comparisons

DisjunctionPredict: { List\<Predict> }

SelectClause: { List\<WithAlias\<Attr>> } // A list of attributes 

FromClause: { List\<Table> }

WhereClause: { Predict } // A general predict, can be disjunction, conjunction...

Query: { SelectClause, FromClause, WhereClause } // WhereClause can be null


### Supported Syntax

Attr -> id

Attr -> id.id

Attr -> (Attr)

Attr -> Attr as id

Table -> id

Table -> id as id

Table -> (Table)

Table -> Table natural join Table natural join ...

SelectClause -> select Attr , Attr , ...

FromClause -> from Table, Table , ...

Comparand -> Attr

Comparand -> id / int / float / 'string'

Operator -> < / <= / = / >= / >

ComparePredict -> Comparand Operator Comparand

ComparePredict -> Comparand between Comparand and Comparand

Predict -> Disjunction

Disjunction -> DisjunctionAtom or DisjunctionAtom or ...

DisjunctionAtom -> Conjunction

DisjunctionAtom -> (Predict)

Conjunction -> ConjunctionAtom and ConjunctionAtom and ...

ConjunctionAtom -> ComparePredict

ConjunctionAtom -> (Predict)

WhereClause -> where Predict

Query -> SelectClause FromClause WhereClause

Query -> SelectClause FromClause



