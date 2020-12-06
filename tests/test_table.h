#ifndef _TEST_TABLE_H_
#define _TEST_TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "value.h"

int test_ht_add_entry() {
    INIT_TEST();
    HashTable *table = init_table();

    char *keys[3];
    char *vals[3];
    for (unsigned int i = 0; i < 3; i++) {
        keys[i] = malloc(sizeof(char) * 5);
        vals[i] = malloc(sizeof(char) * 5);
        snprintf(keys[i], 5, "key%d", i);
        snprintf(vals[i], 5, "val%d", i);

        add_entry(table, keys[i], string_value(vals[i]));
    }

    BEGIN_TEST_CASE("No queried entries are NULL");
    for (unsigned int i = 0; i < 3; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (e == NULL) {
            TEST_FAIL();
            break;
        }

    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("get_entry returns NULL for value not in table");
    if (get_entry(table, "aaaa") != NULL) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Queried entry strings are equal to their intial strings");
    for (unsigned int i = 0; i < 3; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (strcmp(e->value.as.string, vals[i]) != 0) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Deleting key results in get_entry returning NULL for that key");
    del_entry(table, keys[0]);
    if (get_entry(table, keys[0]) != NULL) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Deleting entry that doesn't exist doesn't affect rest of table");
    del_entry(table, "nonexistant key");

    Entry *e1 = get_entry(table, keys[1]);
    Entry *e2 = get_entry(table, keys[2]);

    if (e1 == NULL || e2 == NULL) {
        TEST_FAIL();
    } else {
        if (strcmp(e1->value.as.string, "val1") != 0 || strcmp(e2->value.as.string, "val2") != 0) {
            TEST_FAIL();
        }
    }

    END_TEST_CASE();
    free_table(table);
    for (unsigned int i = 0; i < 3; i++) {
        free(keys[i]);
        free(vals[i]);
    }

    END_TEST();
}

int test_ht_resize() {
    INIT_TEST();
    HashTable *table = init_table();

    int initial_capacity = table->capacity;

    char *keys[8];
    char *vals[8];
    for (unsigned int i = 0; i < 8; i++) {
        keys[i] = malloc(sizeof(char) * 5);
        vals[i] = malloc(sizeof(char) * 5);
        snprintf(keys[i], 5, "key%d", i);
        snprintf(vals[i], 5, "val%d", i);

        add_entry(table, keys[i], string_value(vals[i]));
    }

    BEGIN_TEST_CASE("Table capacity now greater than initial capacity");
    if (table->capacity <= initial_capacity) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("get_entry doesn't return NULL after resize");
    for (unsigned int i = 0; i < 8; i++) {
        Entry *e = get_entry(table, keys[i]);
        if (e == NULL) {
            TEST_FAIL();
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("get_entry returns correct strings after resize");
    for (unsigned int i = 0; i < 8; i++) {
        Entry *e = get_entry(table, keys[i]);
        if (strcmp(e->value.as.string, vals[i]) != 0) {
            TEST_FAIL();
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Deleting entry that doesn't exist doesn't affect table size");
    int prev_capacity = table->capacity;

    del_entry(table, "nonexistant key1");
    del_entry(table, "nonexistant key2");
    del_entry(table, "nonexistant key3");
    del_entry(table, "nonexistant key4");
    del_entry(table, "nonexistant key5");
    del_entry(table, "nonexistant key6");

    if (table->capacity != prev_capacity) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    free_table(table);
    for (unsigned int i = 0; i < 8; i++) {
        free(keys[i]);
        free(vals[i]);
    }

    END_TEST();
}

int test_ht_stress() {
    INIT_TEST();
    HashTable *table = init_table();

    int initial_capacity = table->capacity;

    BEGIN_TEST_CASE("Table keys all NULL on init");
    for (unsigned int i = 0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    unsigned int num_keys = 2000;
    char *keys[num_keys];
    char *vals[num_keys];
    for (unsigned int i = 0; i < num_keys; i++) {
        keys[i] = malloc(sizeof(char) * 8);
        vals[i] = malloc(sizeof(char) * 8);
        snprintf(keys[i], 8, "key%04d", i);
        snprintf(vals[i], 8, "val%04d", i);

        add_entry(table, keys[i], string_value(vals[i]));
    }

    BEGIN_TEST_CASE("Table capacity now greater than initial capacity");
    if (table->capacity <= initial_capacity) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("get_entry succeeds for all keys");
    for (unsigned int i = 0; i < num_keys; i++) {
        if (get_entry(table, keys[i]) == NULL) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Every key now returns NULL after del_entry on all keys");
    for (unsigned int i = 0; i < num_keys; i++) {
        del_entry(table, keys[i]);
    }

    for (unsigned int i = 0; i < num_keys; i++) {
        if (get_entry(table, keys[i]) != NULL) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Table capacity back to initial capacity");
    if (table->capacity != initial_capacity) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Table entries all unoccupied or deleted after del_entry on every key");
    for (unsigned int i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied && !table->entries[i].deleted) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    char *keys2[num_keys];
    for (unsigned int i = 0; i < num_keys; i++) {
        keys2[i] = malloc(sizeof(char) * 12);
        snprintf(keys2[i], 12, "new-key%04d", i);

        add_entry(table, keys2[i], string_value(vals[i]));
    }

    BEGIN_TEST_CASE("get_entry succeeds for all new keys added after deleting all prev. keys");
    for (unsigned int i = 0; i < num_keys; i++) {
        if (get_entry(table, keys2[i]) == NULL) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Every key now returns NULL after del_entry on all keys");
    for (unsigned int i = 0; i < num_keys; i++) {
        del_entry(table, keys2[i]);
    }

    for (unsigned int i = 0; i < num_keys; i++) {
        if (get_entry(table, keys2[i]) != NULL) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Table capacity back to initial capacity");
    if (table->capacity != initial_capacity) {
        TEST_FAIL();
    }
    END_TEST_CASE();

    BEGIN_TEST_CASE("Table entries all unoccupied or deleted after del_entry on every key");
    for (unsigned int i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied && !table->entries[i].deleted) {
            TEST_FAIL();
            break;
        }
    }
    END_TEST_CASE();

    free_table(table);
    for (unsigned int i = 0; i < num_keys; i++) {
        free(keys[i]);
        free(keys2[i]);
        free(vals[i]);
    }

    END_TEST();
}

#endif /* _TEST_TABLE_H_ */
