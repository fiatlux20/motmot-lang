#ifndef _TABLE_H_
#define _TABLE_H_

#include <stdint.h>

#include "objects.h"

typedef struct
{
    uint32_t hash;
    value val;
} key;


typedef struct {
    key k;
    value v;
} entry;

typedef struct {
    unsigned int elements;
    unsigned int capacity;
    entry *entries;
} table;

table init_table(table* table);
void free_table(table* table);

uint32_t hash_string(const char *str);

#endif /* _TABLE_H_ */