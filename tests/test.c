#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "value.h"

/* writing tests:
 *
 * test functions should take no arguments and return an int, e.g.
 * int test_hash_table()
 *
 * a test function should begin with INIT_TEST() and end with END_TEST()
 *
 * within a test function, test cases can be created by enclosing them
 * within BEGIN_TEST_CASE(description) and END_TEST_CASE(). a test case
 * can be signaled to fail with the TEST_FAIL() macro, otherwise when
 * it reaches END_TEST_CASE() the test will be reported as successful
 * e.g.
 * BEGIN_TEST_CASE("x is 1")
 * if (x != 1) TEST_FAIL();
 * END_TEST_CASE();
 *
 * when calling a test function from main, use TEST(testfn, description)
 * to call the test and supply a description of what the test does overall
 */

#define PASSED_TEST 0

#define TEST(testfn, description) \
    do { \
        printf("TEST %d: %s - ", __LINE__, description); \
        testfn(); \
    } while (0)

#define TEST_FAIL() \
    test_code = 1;

#define BEGIN_TEST_CASE(description) \
    do { \
        int line = __LINE__; \
        const char *test_desc = description;

#define END_TEST_CASE() \
    if (test_code == 0) printf("    %d:PASSED %s\n", line, test_desc); \
    else printf("    %d:FAILED %s\n", line, test_desc); \
    test_code = 0; \
    } while (0)

#define INIT_TEST() \
    int test_code = 0; \
    printf("%d:%s\n", __LINE__, __func__);

#define END_TEST() \
    if (test_code == 0) printf("%s PASSED\n", __func__); \
    else printf("%s FAILED\n", __func__); \
    return test_code;

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

    BEGIN_TEST_CASE("Queried entry strings are equal to their intial strings");
    for (unsigned int i = 0; i < 3; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (strcmp(e->value.as.string, vals[i]) != 0) {
            TEST_FAIL();
            break;
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

    BEGIN_TEST_CASE("Table capacity greater than initial capacity");
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

    free_table(table);
    for (unsigned int i = 0; i < 8; i++) {
        free(keys[i]);
        free(vals[i]);
    }

    END_TEST();
}

int main() {
#line 0 /* set line number to 0 to report tests in order */

    TEST(test_ht_add_entry, "Adding 3 entries to a hash table and then retrieving them");
    TEST(test_ht_resize, "Adding 8 entries to a hash table to force a resize and then retrieving them");
}

