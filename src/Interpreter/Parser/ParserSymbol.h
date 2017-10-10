#ifndef __PARSER_SYMBOL__
#define __PARSER_SYMBOL__

namespace ParserSymbol{
    typedef enum{
        identifier = 1,
        str,
        float_,
        int_,
        
        query,
        attr_set,
        attrID,
        attr,
        table_set,
        condition_set,
        condition,
        table,
        tableID
    }Tag;

    typedef enum{
        join = 1,
        naturaljoin,
        parallel,
        dot,
        equality,
        and_,
        or_,
        as
    }Action;

    typedef enum{
        FINISH,
        WAIT_SELECT,
        WAIT_ATTR_ID,
        REDUCE_ATTR_ID,
        WAIT_ATTR_DOT_RIGHT,
        REDUCE_ADDR_ID_WITH_TABLE_ID,
        REDUCE_ATTR,
        WAIT_ATTR_ALIAS,
        REDUCE_ATTR_WITH_ALIAS,
        REDUCE_ATTR_SET,
        WAIT_ATTR_ID_AGAIN,
        WAIT_FROM,
        WAIT_TABLE_ID,
        REDUCE_TABLE_ID,
        RECUDE_TABLE,
        WAIT_TABLE_ALIAS,
        REDUCE_TABLE_WITH_ALIAS,
        REDUCE_TABLE_SET,
        WAIT_WHERE,
        WAIT_CONDITION,
        WAIT_NUM_OR_STR,
        WAIT_EUQALITY,
        REDUCE_CONDITION,
        REDUCE_CONDITION_SET,
        REDUCE_QUERY
    }SLRstate;
}

#endif