#ifndef _TEST_H_
#define _TEST_H_

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

#define PASSED_TEST 0

#define TEST(testfn, description) \
    do { \
        printf("TEST %d: %s - ", __LINE__, description); \
        testfn(); \
    } while (0)

#define TEST_FAIL() \
    test_code = 1; \
    overall_code = 1;

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
    int overall_code = 0; \
    printf("%d:%s\n", __LINE__, __func__);

#define END_TEST() \
    if (overall_code == 0) printf("%s PASSED\n", __func__); \
    else printf("%s FAILED\n", __func__); \
    return overall_code;

#endif /* _TEST_H_ */
