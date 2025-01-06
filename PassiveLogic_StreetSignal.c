/*
 ============================================================================
 Name        : PassiveLogic_StreetSignal.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define nullptr NULL
typedef enum { fail = -1, false, true } bool;
typedef enum {
	eUnknown, eGreen, eYellow, eRed,
	e4WayStop_noTraffic,
	eNormalOperation,
//	eNormalOperation_lightTraffic,
//	eNormalOperation_moderateTraffic,
//	eNormalOperation_heavyTraffic
} LightState;

#define RED_DELAY_DEFAULT 90
#define YELLOW_DELAY_DEFAULT 5

int red_delay = 90;
int yellow_delay = 5;
/*
 * Street signal simulation
 *
 * This is a simulation of traffic light state machine. It will use random() to
 * signal when a vehicle arrives and departs. States:
 * 		1) Init. All lights are red. Goto #1.
 * 		2) If there is a preferred road, choose it, else randomly choose a light to turn green. Goto #2.
 * 		3) If there is a preferred road, wait until vehicle approaches opposite lane before changing; else wait 90 seconds. Goto #4.
 * 		4) Change green signal to yellow. Wait for 5 seconds. Change signal to red. Wait 3 seconds. Change opposite signal to green. Wait 90 seconds. Goto #5.
 * 		5) When a vehicle arrives at the intersection,
 * 			5.a) and if its signal is green, reset counter delay
 * 			5.b) and if its signal is red,
 * 				5.b.i) if there is existing opposite traffic, wait 90 seconds. Execute transition.
 * 				5.b.ii) if there is no opposite traffic, execute transition.
 *
 * 	States:
 * 		- no traffic:
 * 			- with static preference: sit on preference.
 * 			- with alternating preference: sit on the last transition.
 * 			- with no preference: stop sign state, blinking red.
 * 		- light traffic:
 * 			- with static preference: return or stay on preference in between transitions.
 * 			- with alternating preference: stay on current state until opposite traffic arrives.
 * 			- with no preference: sit and hold last transition until arrival.
 * 		- moderate traffic:
 * 			- with static preference: preference gets longer delay.
 * 			- with no preference: transition with shorter delay.
 * 		- heavy traffic:
 * 			- with no preference: transition with longer delay.
 *
 * 	NOTES:
 * 		* I considered implementing this with threads and semaphores, but there's no need and only complicates testing.
 * 		* I believe that left or right turn signals are out of scope.
 */

//LightState street_1 = eUnknown, street_2 = eUnknown, intersection_state = eUnknown;
//int street_1_counter, street_1_counter;
//LightState *preferred = nullptr;
LightState intersectionState = eUnknown;

//=== TOOLS =======================================================================================================================
bool str_startsWith(const char *str, const char *sub) {
	if ( str != nullptr ) {
		if ( sub != nullptr ) {

			while ( *str == *sub  &&  *sub != 0 ) {
				str++, sub++;
			}
			return (*sub == 0);

		} else {
			return true;
		}

	} else {
		return false;
	}
}

bool isBetween(int low, int test, int high) {
	return ( low < test  &&  test < high );
}

void parse_args(const char *args[]) {
	if ( args != nullptr ) {
		while ( *args != nullptr ) {
			if ( str_startsWith(*args, "--red-delay=") ) {
				const char *seconds = *args + strlen("--red-delay=");
				red_delay = ( atoi(seconds)? : RED_DELAY_DEFAULT );

			} else if ( str_startsWith(*args, "--yellow-delay=") ) {
				const char *seconds = *args + strlen("--yellow-delay=");
				yellow_delay = ( atoi(seconds)? : YELLOW_DELAY_DEFAULT );

//			} else if ( strcmp(*args, "--preferred=main") == 0 ) {
//				preferred = &centerStreet;
//
//			} else if ( strcmp(*args, "--preferred=center") == 0 ) {
//				preferred = &mainStreet;
			}

			args++;
		}

	} else {
		fprintf(stderr, "Command line args are NULL. Not possible!\n");
		abort();
	}
}

//=== STATES ======================================================================================================================
void init(void) {
//	centerStreet = eRed;
//	mainStreet = eRed;
	intersectionState = e4WayStop_noTraffic;
}

//=== PROGRAM ENTRY ===============================================================================================================
bool run = true;

//typedef struct {
	int mainStreet_carCounter;
	int centerStreet_carCounter;
//} IntersectionState;


void state_loop(void) {
	int state_step = 0;

	while ( run ) {
		switch ( intersectionState ) {
			case e4WayStop_noTraffic:
				if ( (state_step & 4) == 0 ) {
					printf("Main Street's red light turns on. Center Street's red light turns off.\n");
				}
				if ( (state_step & 4) != 0 ) {
					printf("Main Street's red light turns off. Center Street's red light turns on.\n");
				}
			//TODO: what if the cars start lining up?
				break;

			case eNormalOperation://_lightTraffic:

				break;

			default:
				fprintf(stderr, "ERROR! Cannot get here!\n");
				abort();
		}
		sleep(1);
		if ( isBetween(100, random() % 1000, 200) ) {
			mainStreet_carCounter++;
		}
		if ( isBetween(400, random() % 1000, 500) ) {
			centerStreet_carCounter++;
		}
	}
}

int main(int cnt, const char *args[]) {
	if ( strcasecmp(args[1], "test") == 0 ) {
		int run_tests(void);
		return run_tests();

	} else {
		parse_args(++args);
		init();

		return EXIT_SUCCESS;
	}
}

//=== UNIT TESTS ==================================================================================================================
#define _RED_ "\x1B[31m"
#define _UNDERLINE_ "\x1B[4m"
#define _RESET_ "\x1B[0m"
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
					_RED_ "FAILED. Expected "
					_UNDERLINE_ "%d" _RESET_
					_RED_ " but got "
					_UNDERLINE_ "%d" _RESET_
					_RED_ "."
					_RESET_ "\n",
				file, function, linenum, expected_text, got_text, expected, got);
		fflush(stderr);
	}
}
void test_str_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, char *expected, char *got) {
	if ( strcmp(expected, got) == 0 ) {
		passed++;
		fprintf(stdout, "%s:%s[%d]: %s ≟ %s... PASSED.\n", file, function, linenum, expected_text, got_text);
		fflush(stdout);

	} else {
		failed++;
		fprintf(stderr,
				"%s:%s[%d]: %s ≟ %s... "
					_RED_ "FAILED. Expected "
					_UNDERLINE_ "%s" _RESET_
					_RED_ " but got "
					_UNDERLINE_ "%s" _RESET_
					_RED_ "."
					_RESET_ "\n",
				file, function, linenum, expected_text, got_text, expected, got);
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
					_RED_ "FAILED"
					_RESET_ "\n",
				file, function, linenum, test_text);
		fflush(stderr);
	}
}

#define SEW_TEST_ASSERT(test) test_assert(__FILE__, __PRETTY_FUNCTION__, __LINE__, #test, test)
#define SEW_TEST_INT_EQUALS(expected, result) test_int_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (long)result, (long)expected)
#define SEW_TEST_STR_EQUALS(expected, result) test_str_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (long)result, (long)expected)
void test__parse_args(void) {
	{	const char *args[] = {nullptr};
		red_delay = -1;
		yellow_delay = -1;
//		centerStreet = eUnknown;
//		mainStreet = eUnknown;
		intersectionState = eUnknown;

		parse_args(args);

		SEW_TEST_INT_EQUALS(red_delay, -1);
		SEW_TEST_INT_EQUALS(yellow_delay, -1);
//		SEW_TEST_INT_EQUALS(centerStreet, eUnknown);
//		SEW_TEST_INT_EQUALS(mainStreet, eUnknown);
		SEW_TEST_INT_EQUALS(intersectionState, eUnknown);
//		SEW_TEST_EQUALS(preferred, nullptr);
	}
	{
		const char *args[] = { "--preferred=center", "--red-delay=100", "--yellow-delay=200" };

		red_delay = -1;
		yellow_delay = -1;
//		centerStreet = eUnknown;
//		mainStreet = eUnknown;
		intersectionState = eUnknown;

		parse_args(args);

		SEW_TEST_INT_EQUALS(red_delay, 100);
		SEW_TEST_INT_EQUALS(yellow_delay, 200);
//		SEW_TEST_INT_EQUALS(centerStreet, eUnknown);
//		SEW_TEST_EQUALS(mainStreet, eUnknown);
		SEW_TEST_INT_EQUALS(intersectionState, eUnknown);
//		SEW_TEST_INT_EQUALS(preferred, centerStreet);
	}
	{
		const char *args[] = { "--preferred=main", "--red-delay=1", "--yellow-delay=2" };

		red_delay = -1;
		yellow_delay = -1;
//		centerStreet = eUnknown;
//		mainStreet = eUnknown;
		intersectionState = eUnknown;

		parse_args(args);

		SEW_TEST_INT_EQUALS(red_delay, 1);
		SEW_TEST_INT_EQUALS(yellow_delay, 2);
//		SEW_TEST_INT_EQUALS(centerStreet, eUnknown);
//		SEW_TEST_INT_EQUALS(mainStreet, eUnknown);
		SEW_TEST_INT_EQUALS(intersectionState, eUnknown);
//		SEW_TEST_INT_EQUALS(preferred, mainStreet);
	}
}

void test__str_startsWith(void) {
	SEW_TEST_ASSERT(!str_startsWith(nullptr, nullptr));
	SEW_TEST_ASSERT(!str_startsWith(nullptr, ""));
	SEW_TEST_ASSERT(str_startsWith("", nullptr));
	SEW_TEST_ASSERT(str_startsWith("", ""));
	SEW_TEST_ASSERT(str_startsWith("a", nullptr));
	SEW_TEST_ASSERT(str_startsWith("abc", ""));
	SEW_TEST_ASSERT(str_startsWith("abc", "a"));
	SEW_TEST_ASSERT(str_startsWith("abc", "ab"));
	SEW_TEST_ASSERT(!str_startsWith("abc", "ax"));
	SEW_TEST_ASSERT(!str_startsWith("abc", "bc"));
	SEW_TEST_ASSERT(!str_startsWith("abc", "x"));
}

void test_isBetween(void) {
	SEW_TEST_ASSERT(!isBetween(0,0,0));
	SEW_TEST_ASSERT(!isBetween(0,0,10));
	SEW_TEST_ASSERT(!isBetween(0,10,0));
	SEW_TEST_ASSERT(!isBetween(0,10,10));
	SEW_TEST_ASSERT(!isBetween(10,0,0));
	SEW_TEST_ASSERT(!isBetween(10,0,10));
	SEW_TEST_ASSERT(!isBetween(10,10,0));
	SEW_TEST_ASSERT(!isBetween(10,10,10));

	SEW_TEST_ASSERT(isBetween(0,5,10));
	SEW_TEST_ASSERT(isBetween(0,1,10));
	SEW_TEST_ASSERT(isBetween(0,9,10));
}

int run_tests(void) {
	test__parse_args();
	test__str_startsWith();
	fprintf((failed > 0? stderr: stdout), "%d/%d tests failed\n", failed, failed + passed);
	return 0;
}











