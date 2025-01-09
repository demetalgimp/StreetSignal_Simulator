/*
 * unittester.h
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */

#ifndef UNITTESTER_H_
#define UNITTESTER_H_

extern int passed, failed;

void test_int_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, int expected, int got);
void test_str_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, const char *expected, const char *got);
void test_assert(const char *file, const char *function, int linenum, const char *test_text, int test);

#define TEST_ASSERT(test) test_assert(__FILE__, __PRETTY_FUNCTION__, __LINE__, #test, test)
#define TEST_INT_EQUALS(expected, result) test_int_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (long)result, (long)expected)
#define TEST_STR_EQUALS(expected, result) test_str_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (const char*)result, (const char*)expected)

#endif /* UNITTESTER_H_ */
