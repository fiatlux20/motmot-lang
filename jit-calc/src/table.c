#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "function.h"
#include "table.h"

/* value functions */

#define FNV1_32_INIT 2166136261u
#define FNV1_32_PRIME 16777619u

static uint32_t hash_string_n(const char *string, unsigned int length) {
    static const uint32_t prime = FNV1_32_PRIME;
    uint32_t hash = FNV1_32_INIT;

    for (unsigned int i = 0; i < length; i++) {
        hash ^= string[i];
        hash *= prime;
    }

    return hash;
}

static uint32_t hash_string(const char *string) {
    return hash_string_n(string, strlen(string));
}

static void resize_table(HashTable *table, unsigned int new_size) {
    if (table->elements > new_size) {
        printf("bad\n");
    }

    Entry *old_entries = table->entries;
    table->entries = calloc(new_size, sizeof *table->entries);

    unsigned int old_capacity = table->capacity;
    table->capacity = new_size;
    table->elements = 0;

    for (unsigned int i = 0; i < old_capacity; i++) {
        if (old_entries[i].occupied && !old_entries[i].deleted) {
            add_entry(table, old_entries[i].key, old_entries[i].value);
        }
    }

    free(old_entries);
    old_entries = NULL;
}

/* public functions */
HashTable *create_hash_table() {
    HashTable *table = malloc(sizeof *table);

    table->entries = calloc(TABLE_DEFAULT_SIZE, sizeof *table->entries);
    table->elements = 0;
    table->capacity = TABLE_DEFAULT_SIZE;

    return table;
}

void init_hash_table(HashTable *table) {
    table->entries = calloc(TABLE_DEFAULT_SIZE, sizeof *table->entries);
    table->elements = 0;
    table->capacity = TABLE_DEFAULT_SIZE;
}

void add_entry(HashTable *table, char *key, Function *value) {
    if ((table->elements + 1) * 100 / table->capacity > 70) {
        resize_table(table, table->capacity * 2);
    }
    const uint32_t hash = hash_string(key);
    unsigned int index = hash & (table->capacity - 1);

    while (table->entries[index].occupied && !table->entries[index].deleted) {
        index = (index + 1) & (table->capacity - 1);
    }

    table->entries[index] = (Entry) { key, value, 1, 0 };
    table->elements++;
}

static Entry *get_entry(HashTable *table, char *key) {
    const uint32_t hash = hash_string(key);
    unsigned int index = hash & (table->capacity - 1);
    unsigned int moveEntry = 0;
    unsigned int moveIndex = 0;

    while (table->entries[index].key != key) {
        if (!table->entries[index].occupied) {
            return NULL;
        }

        if (!moveEntry && table->entries[index].deleted) {
            moveEntry = 1;
            moveIndex = index;
        }

        index = (index + 1) & (table->capacity - 1);
    }

    if (table->entries[index].deleted) {
        return NULL;
    }

    if (moveEntry) { /* perform lazy move */
        table->entries[moveIndex] = table->entries[index];
        table->entries[index].deleted = 1;
        return &table->entries[moveIndex];
    } else { 
        return &table->entries[index];
    }
}

Function *get_entry_value(HashTable *table, char *key) {
    Entry *e = get_entry(table, key);
    if (e != NULL) {
        return e->value;
    } else {
        return NULL;
    }
}

char *get_key_by_string_n(HashTable *table, char *key, unsigned int length) {
    const uint32_t hash = hash_string_n(key, length);
    unsigned int index = hash & (table->capacity - 1);

    while (table->entries[index].key != NULL && strncmp(table->entries[index].key, key, length) != 0) {
        if (!table->entries[index].occupied) {
            return NULL;
        }

        index = (index + 1) & (table->capacity - 1);
    }

    return table->entries[index].key;
}


void del_entry(HashTable *table, char *key) {
    if (table->capacity > TABLE_DEFAULT_SIZE && table->elements * 100 / table->capacity < 15) {
        resize_table(table, table->capacity / 2);
    }

    Entry *e = get_entry(table, key);
    if (e != NULL) {
        e->deleted = 1;
    }

    table->elements--;
}

void free_hash_table(HashTable *table) {
    free(table->entries);
    free(table);
    table = NULL;
}

#ifdef DEBUG_TABLE
void print_table(HashTable *table) {
    for (unsigned int i = 0; i < table->capacity; i++) {
        printf("%s:  ", table->entries[i].key);
        print_value(&table->entries[i].value);
        printf("\n");
    }
}
#endif /* DEBUG_TABLE */
