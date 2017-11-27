#ifndef __PARSER_SYMBOL__
#define __PARSER_SYMBOL__

#define QUERY_STATE_CNT 25
#define DELETE_STATE_CNT 12
#define INSERT_STATE_CNT 9
#define CREATE_TABLE_STATE_CNT 11
#define CREATE_INDEX_STATE_CNT 8
#define UPDATE_STATE_CNT 17

namespace ParserSymbol{
    typedef enum{
        none,
        
        identifier,
        str_,
        float_,
        int_,
        
        query_,
        delete_,
        insert_,
        update_,
        create_table_,
        drop_table,
        drop_index,
        create_index,

        attr_set,
        attrID,
        attr,
        table_set,
        condition_set,
        condition,
        table,
        tableID,

        type,
        constrain,
        meta,
        meta_set,
        assign,
        assign_set,
        value_set
    }Tag;

    typedef enum{
        parallel = 1,

        join,
        naturaljoin,
        dot,

        equal_,
        less_,
        larger_,
        larger_equal_,
        less_equal_,
        not_equal_,

        and_,
        or_,

        as,

        star,

        type_int,
        type_float,
        type_char,
        type_varchar,

        primary_key,
        not_null,
        unique_,
    }Action;

    typedef enum{
        //select
        FINISH_QUERY,
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
        REDUCE_QUERY_WITHOUT_CONDITION,
        WAIT_CONDITION,
        WAIT_NUM_OR_STR,
        WAIT_EQUALITY,
        REDUCE_CONDITION,
        REDUCE_CONDITION_SET,
        REDUCE_QUERY_WITH_CONDITION
    }QueryState;

    typedef enum{
        FINISH_DELETE,
        WAIT_DELETE,
        WAIT_FROM_IN_DELETE,
        WAIT_TABLE_ID_IN_DELETE,
        WAIT_WHERE_IN_DELETE,
        WAIT_CONDITION_IN_DELETE,
        WAIT_ATTR_IN_DELETE,
        WAIT_NUM_OR_STR_IN_DELETE,
        WAIT_EQUALITY_IN_DELETE,
        REDUCE_CONDITION_IN_DELETE,
        REDUCE_CONDITION_SET_IN_DELETE,
        REDUCE_DELETE,
    }DeleteState;

    typedef enum{
        FINISH_INSERT,
        WAIT_INSERT,
        WAIT_INTO,
        WAIT_TABLE_IN_INSERT,
        WAIT_VALUE_SET,
        BEGIN_OF_VALUE_SET,
        WAIT_SINGLE_VALUE,
        REDUCE_VALUE_SET,
        REDUCE_INSERT,
    }InsertState;

    typedef enum{
        FINISH_CREATE_TABLE,
        WAIT_TABLE_IN_CREATE_TABLE,
        BEGIN_OF_META_SET,
        WAIT_META,
        WAIT_TYPE,
        WAIT_TYPE_PARAM,
        REDUCE_TYPE,
        WAIT_CONSTRAIN,
        REDUCE_META,
        REDUCE_META_SET,
        REDUCE_CREATE_TABLE,
    }CreateTableState;

    typedef enum{
        FINISH_CREATE_INDEX,
        WAIT_INDEX_NAME,
        WAIT_ON,
        WAIT_TABLE_IN_CREATE_INDEX,
        BEGIN_OF_COLUMN_SET,
        WAIT_COLUMN_SET,
        REDUCE_COLUMN_SET,
        REDUCE_CREATE_INDEX
    }CreateIndexState;

    typedef enum{
        FINISH_UPDATE,
        WAIT_UPDATE,
        WAIT_TABLE_IN_UPDATE,
        BEGIN_OF_ASSIGN_SET,
        WAIT_ATTR_IN_ASSIGN,
        WAIT_EQUAL_IN_ASSIGN,
        WAIT_ASSIGN_VALUE,
        REDUCE_ASSIGN,
        REDUCE_ASSIGN_SET,
        WAIT_WHERE_IN_UPDATE,
        WAIT_CONDITION_IN_UPDATE,
        WAIT_ATTR_IN_UPDATE,
        WAIT_NUM_OR_STR_IN_UPDATE,
        WAIT_EQUALITY_IN_UPDATE,
        REDUCE_CONDITION_IN_UPDATE,
        REDUCE_CONDITION_SET_IN_UPDATE,
        REDUCE_UPDATE
    }UpdateState;

}

#endif