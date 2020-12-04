#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "value.h"

#define PASSED_TEST 0

#define TEST(testfn, description) \
    do { \
        int ret = 0; \
        if ((ret = testfn()) == PASSED_TEST) printf("%d: PASSED: %s\n", __LINE__, description); \
        else printf("%d: FAILED: %s\n    TEST RETURNED: %d\n", __LINE__, description, ret); \
    } while (0);

#define TEST_EQUALS_INT(testfn, expected, description) \
    do { \
        int ret = 0; \
        if ((ret = testfn()) == expected) printf("PASSED: %s\n", description); \
        else printf("FAILED: %s\n    EXPECTED: %d, GOT: %d\n", description, expected, ret); \
    } while (0);

int test_ht_add_entry() {
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

    for (unsigned int i = 0; i < 3; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (e == NULL) {
            return 1;
        }

        if (strcmp(e->value.as.string, vals[i]) != 0) {
            return 2;
        }
    }

    free_table(table);
    for (unsigned int i = 0; i < 3; i++) {
        free(keys[i]);
        free(vals[i]);
    }

    return PASSED_TEST;
}

int test_ht_resize() {
    HashTable *table = init_table();

    char *keys[8];
    char *vals[8];
    for (unsigned int i = 0; i < 8; i++) {
        keys[i] = malloc(sizeof(char) * 5);
        vals[i] = malloc(sizeof(char) * 5);
        snprintf(keys[i], 5, "key%d", i);
        snprintf(vals[i], 5, "val%d", i);

        add_entry(table, keys[i], string_value(vals[i]));
    }

    if (table->capacity <= 8) {
        return 3; /* table didn't resize itself */
    }

    for (unsigned int i = 0; i < 8; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (e == NULL) {
            return 1; /* got null while querying for key */
        }

        if (strcmp(e->value.as.string, vals[i]) != 0) {
            return 2; /* value returned not equal to original value */
        }
    }

    free_table(table);
    for (unsigned int i = 0; i < 8; i++) {
        free(keys[i]);
        free(vals[i]);
    }

    return PASSED_TEST;
}

int main() {
#line 0 /* set line number to 0 to report tests in order */

    TEST(test_ht_add_entry, "Adding 3 entries to a hash table and then retrieving them");
    TEST(test_ht_resize, "Adding 8 entries to a hash table to force a resize and then retrieving them");
}

