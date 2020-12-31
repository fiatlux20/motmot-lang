/** @file table.h
 * Hash table functions and data structures.
 */

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

typedef struct Entry {
    char *key;
    Value value;
    unsigned char occupied;
    unsigned char deleted;
} Entry;

typedef struct HashTable {
    Entry *entries;    
    unsigned int elements;
    unsigned int capacity;
} HashTable;

/**
 * Heap-allocates memory for a hash table and initializes it. Must be freed with
 * free_table().
 *
 * @return An initialized hash table.
 */
HashTable *init_table();

/**
 * Frees the memory used by a hash table and sets the pointer to NULL.
 *
 * @param table The hash table to be freed.
 */
void free_table(HashTable *table);

/**
 * Adds a key and value to the hash table. When retrieving the value later, the
 * table compares the pointer rather than the string, so the pointer to key must
 * never be lost to be able to later retrieve the value.
 *
 * @param table The table to add the key-value pair to.
 * @param key A string identifying the value.
 * @param value The value to be associated with the key.
 */
void add_entry(HashTable *table, char *key, Value value);

void update_entry(HashTable *table, char *key, Value value);

/**
 * Looks for a key in the table, and returns a pointer to the entry if it is found,
 * or a NULL if it is not found.
 *
 * @param table The table to search for key in.
 * @param key The key the value is associated with.
 * @return A pointer to the entry on success, or NULL on failure.
 */
Entry *get_entry(HashTable *table, char *key);

/**
 * Deletes a key-value pair from the table.
 *
 * @param table The table to delete key from.
 * @param key The key to a value to be deleted.
 */
void del_entry(HashTable *table, char *key);
#ifdef DEBUG_TABLE
void print_table(HashTable *table);
#endif /* DEBUG_TABLE */

#endif /* _TABLE_H_*/
