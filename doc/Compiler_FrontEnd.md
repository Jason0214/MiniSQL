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

Parsers for this SQL project is still under testing