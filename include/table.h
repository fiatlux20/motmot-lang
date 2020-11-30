#ifndef _TABLE_H_
#define _TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "common.h"
#include "value.h"

#define TABLE_DEFAULT_SIZE 8
#define HEAP_ALLOCD

typedef struct hash_table HashTable;
typedef struct entry Entry;


struct entry {
    char *key;
    Value value;
    unsigned char occupied;
    unsigned char deleted;
};

struct hash_table {
    Entry *entries;    
    unsigned int elements;
    unsigned int capacity;
};

HashTable *init_table();
void free_table(HashTable *table);

void add_entry(HashTable *table, char *key, Value value);
void update_entry(HashTable *table, char *key, Value value);
Entry *get_entry(HashTable *table, char *key);
void del_entry(HashTable *table, char *key);

#endif /* _TABLE_H_*/
