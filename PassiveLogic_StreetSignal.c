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
#include <string.h>

#define nullptr NULL
typedef enum { fail = -1, false, true } bool;
typedef enum { eUnknown, eGreen, eYellow, eRed, e4WayStop} LightState;
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
 */

LightState street_1 = eUnknown, street_2 = eUnknown, intersection_state = eUnknown;
int street_1_counter, street_1_counter;
LightState *preferred = nullptr;

//=== TOOLS =======================================================================================================================
bool str_startsWith(const char *str, const char *sub) {
	if ( str != nullptr ) {
		if ( sub != nullptr ) {

			while ( *str == *sub ) {
				str++, sub++;
			}
			return (*sub == 0);

		} else {
			return true;
		}

	} else {
		return ( sub == nullptr);
	}

}

//=== STATES ======================================================================================================================
void init(void) {
	street_1 = eRed;
	street_2 = eRed;
	intersection_state = e4WayStop;
}

//=== PROGRAM ENTRY ===============================================================================================================
#define RED_DELAY_DEFAULT 90
#define YELLOW_DELAY_DEFAULT 5

int red_delay = 90;
int yellow_delay = 5;

void parse_args(const char *args[]) {
	if ( args != nullptr ) {
		while ( *args != nullptr ) {
			if ( strcmp(*args, "--preferred=street1") ) {
				preferred = &street_1;

			} else if ( strcmp(*args, "--preferred=street2") ) {
				preferred = &street_2;

			} else if ( str_startsWith(*args, "--red-delay=") ) {
				red_delay = ( atoi(*args + strlen("--red-delay"))? : RED_DELAY_DEFAULT );

			} else if ( str_startsWith(*args, "--yellow-delay=") ) {
				yellow_delay = ( atoi(*args + strlen("--yellow-delay"))? : YELLOW_DELAY_DEFAULT );
			}
			args++;
		}
	} else {
		fprintf(stderr, "Command line args are NULL. Not possible!\n");
		abort();
	}
}

int run_tests(void);

int main(int cnt, const char *args[]) {
//	if ( strcasecmp(args[1], "runtests") == 0 ) {
		return run_tests();
//
//	} else {
//		parse_args(++args);
//		return EXIT_SUCCESS;
//	}
}

//=== UNIT TESTS ==================================================================================================================
void test_equals(const char *file, const char *function, int linenum, const char *expected_text, const char *got_text, int expected, int got) {
	if ( expected == got ) {
		fprintf(stdout, "%s:%s[%d]: %s ≟ %s... PASSED.\n", file, function, linenum, expected_text, got_text);
		fflush(stdout);

	} else {
		fprintf(stderr, "%s:%s[%d]: %s ≟ %s... \x1B[31mFAILED. Expected \x1B[4m%d\x1B[0m but got \x1B[4m%d\x1B[0m\x1B[31m.\x1B[0m\n", file, function, linenum, expected_text, got_text, expected, got);
	}
}
#define NUM_2_STR(num) num
#define SEW_TEST_EQUALS(expected, result) test_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (long)result, (long)expected)
void test__parse_args(void) {
	{	const char *args[] = {nullptr};
		red_delay = -1;
		yellow_delay = -1;
		street_1 = eUnknown;
		street_2 = eUnknown;
		intersection_state = eUnknown;

		parse_args(args);

		SEW_TEST_EQUALS(red_delay, -1);
		SEW_TEST_EQUALS(yellow_delay, -1);
		SEW_TEST_EQUALS(street_1, eUnknown);
		SEW_TEST_EQUALS(street_2, eUnknown);
		SEW_TEST_EQUALS(intersection_state, eUnknown);
		SEW_TEST_EQUALS(preferred, nullptr);
	}
	{
		const char *args[] = { "--preferred=street1", "--red-delay=100", "--yellow-delay=200" };

		red_delay = -1;
		yellow_delay = -1;
		street_1 = eUnknown;
		street_2 = eUnknown;
		intersection_state = eUnknown;

		parse_args(args);

		SEW_TEST_EQUALS(red_delay, 100);
		SEW_TEST_EQUALS(yellow_delay, 200);
		SEW_TEST_EQUALS(street_1, eUnknown);
		SEW_TEST_EQUALS(street_2, eUnknown);
		SEW_TEST_EQUALS(intersection_state, eUnknown);
		SEW_TEST_EQUALS(preferred, street_1);
	}
	{
		const char *args[] = { "--preferred=street2", "--red-delay=1", "--yellow-delay=2" };

		red_delay = -1;
		yellow_delay = -1;
		street_1 = eUnknown;
		street_2 = eUnknown;
		intersection_state = eUnknown;

		parse_args(args);

		SEW_TEST_EQUALS(red_delay, 1);
		SEW_TEST_EQUALS(yellow_delay, 2);
		SEW_TEST_EQUALS(street_1, eUnknown);
		SEW_TEST_EQUALS(street_2, eUnknown);
		SEW_TEST_EQUALS(intersection_state, eUnknown);
		SEW_TEST_EQUALS(preferred, street_2);
	}
}

int run_tests(void) {
	test__parse_args();
	return 0;
}











