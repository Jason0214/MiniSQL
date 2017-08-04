#ifndef _CONSTANT_H_
#define _CONSTANT_H_

typedef enum {
	DB_RECORD_BLOCK = 1,
	DB_SCHEMA_BLOCK,
	DB_BPNODE_BLOCK,
	DB_USER_BLOCK,
	DB_DATABASE_BLOCK,
	DB_DELETED_BLOCK,
	DB_TABLE_BLOCK,
	DB_BPTREE_INDEX,

	DB_TYPE_CHAR = 100,
	DB_TYPE_INT = 356,
	DB_TYPE_FLOAT = 357,

	DB_TEMPORAL_TABLE,
	DB_MATERIALIZED_TABLE,
	DB_ONDISC_TABLE,
	
	DB_DOUBLE_BLOCK_NESTED_JOIN,
	DB_SINGLE_BLOCK_NESTED_JOIN,
	DB_NESTED_LOOP_JOIN,
}DBenum;

// STATIC_ASSERT(sizeof(enum e_DBenum) == 1);

#endif