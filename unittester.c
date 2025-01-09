//tabstop=4
/*
 * unittester.c
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */
#include <stdio.h>
#include <string.h>
#include "unittester.h"

#define nullptr NULL

#define _RED_       "\x1B[31m"
#define _YELLOW_    "\x1B[93m"
#define _GREEN_     "\x1B[32m"
#define _UNDERLINE_ "\x1B[4m"
#define _RESET_     "\x1B[0m"

int passed = 0, failed = 0;

void test_int_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, int expected, int got) {
	if ( expected == got ) {
		passed++;
		fprintf(stdout, "%s:%s[%d]: %s ≟ %s... PASSED.\n", file, function, linenum, expected_text, got_text);
		fflush(stdout);

	} else {
		failed++;
		fprintf(stderr,
				"%s:%s[%d]: %s ≟ %s... "
					_RED_ "FAILED. Expected " _UNDERLINE_ "%d" _RESET_ _RED_ " but got " _UNDERLINE_ "%d" _RESET_ _RED_ "." _RESET_ "\n",
				file, function, linenum, expected_text, got_text, expected, got);
		fflush(stderr);
	}
}

void test_str_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, const char *expected, const char *got) {
	if ( strcmp(expected, got) == 0 ) {
		passed++;
		fprintf(stdout, "%s:%s[%d]: %s ≟ %s... PASSED.\n", file, function, linenum, expected_text, got_text);
		fflush(stdout);

	} else {
		failed++;
		char nl = (strchr(expected, '\n') != nullptr? '\n': '"');
		fprintf(stderr,
				"%s:%s[%d]: %s ≟ %s... "
					_RED_ "FAILED. Expected " _RESET_ "%c%s" _RED_ _RESET_ "%c" _RED_ " but got " _RESET_ "%c%s%c" _RED_ "." _RESET_ "\n",
				file, function, linenum, expected_text, got_text, nl, expected, nl, nl, got, nl);
		fflush(stderr);
	}
}

void test_assert(const char *file, const char *function, int linenum, const char *test_text, int test) {
	if ( test ) {
		passed++;
		fprintf(stdout, "%s:%s[%d]: %s?... PASSED.\n", file, function, linenum, test_text);
		fflush(stdout);

	} else {
		failed++;
		fprintf(stderr,
				"%s:%s[%d]: %s?... "
					_RED_ "FAILED" _RESET_ "\n",
				file, function, linenum, test_text);
		fflush(stderr);
	}
}



