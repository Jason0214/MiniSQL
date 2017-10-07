#ifndef __PARSER_SYMBOL__
#define __PARSER_SYMBOL__

namespace ParserSymbol{
    typedef enum{
        identifier,
        str,
        floats,
        ints,
        
        query,
        attr_set,
        valid_tables,
        attrID,
        attr,
        table_set,
        condition_set,
        condition,
        table
    }Tag;

    typedef enum{
        join,
        naturaljoin,
        comma,
        dot,
        single,
        as
    }Action;

    typedef enum{

    }State;
}

#endif