#ifndef _CONSTANT_H_
#define _CONSTANT_H_

typedef enum {
	DB_TEMP_BLOCK = 1,
	DB_SCHEMA_BLOCK,
	DB_USER_BLOCK,
	DB_DATABASE_BLOCK,
	DB_DELETED_BLOCK,
	
}DBenum;

// STATIC_ASSERT(sizeof(enum e_DBenum) == 1);

#endif