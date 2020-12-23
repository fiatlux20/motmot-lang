#ifndef _TABLE_H_
#define _TABLE_H_

#define TABLE_DEFAULT_SIZE 8

typedef struct function Function;

typedef struct entry {
    char *key;
    Function *value;
    unsigned char occupied;
    unsigned char deleted;
} Entry;

typedef struct hash_table {
    Entry *entries;    
    unsigned int elements;
    unsigned int capacity;
} HashTable;

HashTable *create_hash_table();
void init_hash_table(HashTable *table);
void free_hash_table(HashTable *table);

void add_entry(HashTable *table, char *key, Function *value);
void del_entry(HashTable *table, char *key);
Function *get_entry_value(HashTable *table, char *key);
char *get_key_by_string_n(HashTable *table, char *key, unsigned int length);

#ifdef DEBUG_TABLE
void print_table(HashTable *table);
#endif /* DEBUG_TABLE */

#endif /* _TABLE_H_*/
