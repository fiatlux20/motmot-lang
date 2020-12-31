#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "test_table.h"
#include "test_component.h"

/* writing tests:
 *
 * tests should be defined in headers and included in test.c
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


int main() {
#line 1 /* set line number to 1 to report tests in order */
    TEST(test_ht_add_entry, "Adding 3 entries to a hash table and then retrieving them");
    TEST(test_ht_resize, "Adding 8 entries to a hash table to force a resize and then retrieving them");
    TEST(test_ht_stress, "Add 2000 entries, delete 2000 entries, add 2000 new entries");
    TEST(test_motmot_arithmetic, "Source strings compile to expected bytecode and evaluate to expected result");
}

