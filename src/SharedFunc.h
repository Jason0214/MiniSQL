#ifndef _SHARED_FUNC_
#define _SHARED_FUNC_
#include "CONSTANT.h"

size_t typeLen(DBenum type);

int compare(const void* v1, const void* v2, DBenum type);

void printByType(const void* v, DBenum type);

#endif