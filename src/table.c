#include "table.h"

/* value functions */

#define FNV1_32_INIT 2166136261u
#define FNV1_32_PRIME 16777619u

static uint32_t hash_string(const char *string) {
    static const uint32_t prime = FNV1_32_PRIME;
    uint32_t hash = FNV1_32_INIT;
    unsigned int len = strlen(string);

    for (unsigned int i = 0; i < len; i++) {
        hash ^= string[i];
        hash *= prime;
    }

    return hash;
}

static void resize_table(HashTable *table, unsigned int new_size) {
    if (table->elements > new_size) {
        printf("bad\n");
    }

    Entry *old_entries = table->entries;
    table->entries = calloc(new_size, sizeof *table->entries);

    unsigned int old_capacity = table->capacity;
    unsigned int old_elements = table->elements;
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
HashTable *init_table() {
    HashTable *table = malloc(sizeof *table);

    table->entries = calloc(TABLE_DEFAULT_SIZE, sizeof *table->entries);
    table->elements = 0;
    table->capacity = TABLE_DEFAULT_SIZE;

    return table;
}

void add_entry(HashTable *table, char *key, Value value) {
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

Entry *get_entry(HashTable *table, char *key) {
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

void update_entry(HashTable *table, char *key, Value value) {
    Entry *e = get_entry(table, key);
    if (e != NULL) {
        if (e->deleted) {
            add_entry(table, key, value);
        } else {
            e->value = value;
        }
    }
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

void free_table(HashTable *table) {
#ifdef HEAP_ALLOCD
    for (unsigned int i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied) {
            // free(table->entries[i].key);
            if (table->entries[i].value.type == VAL_TYPE_STRING) {
                free(table->entries[i].value.as.string);
            }
        }
    }
#endif /* HEAP_ALLOCD */

    free(table->entries);
    table->entries = NULL;

    free(table);
    table = NULL;
}
