#ifndef __PARSER_SYMBOL__
#define __PARSER_SYMBOL__

namespace ParserSymbol{
    typedef enum{
        none,
        
        identifier,
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
        equal_,
        less_,
        larger_,
        larger_equal,
        less_equal,
        not_equal,
        and_,
        or_,
        as,
        star
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
        WAIT_FROM,
        WAIT_TABLE_ID,
        REDUCE_TABLE_ID,
        REDUCE_TABLE,
        WAIT_TABLE_ALIAS,
        REDUCE_TABLE_WITH_ALIAS,
        REDUCE_TABLE_SET,
        WAIT_WHERE,
        REDUCE_QUERY_WITHOUT_CONDOTION,
        WAIT_CONDITION,
        WAIT_NUM_OR_STR,
        WAIT_EQUALITY,
        REDUCE_CONDITION,
        REDUCE_CONDITION_SET,
        REDUCE_QUERY_WITH_CONDITION
    }SLRstate;
}

#endif