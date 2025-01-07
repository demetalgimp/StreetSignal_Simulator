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

#define GREEN_DURATION_DEFAULT 90
#define YELLOW_DURATION_DEFAULT 5

#define _RED_       "\x1B[31m"
#define _YELLOW_    "\x1B[93m"
#define _GREEN_     "\x1B[32m"
#define _UNDERLINE_ "\x1B[4m"
#define _RESET_     "\x1B[0m"

bool str_startsWith(const char *str, const char *sub);
bool isBetween(int low, int test, int high);
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
//LightState intersectionState = eUnknown;

//=== STATES ======================================================================================================================
typedef enum { eGreen, eYellow, eRed, eOff } SemaphoreColor;
typedef enum {
	eUnknown,
	e4WayStop_noTraffic,
	eNormalOperation,
	e4Way_to_NormalOperation,
	eCountdown,
//	eNormalOperation_lightTraffic,
//	eNormalOperation_moderateTraffic,
//	eNormalOperation_heavyTraffic
} LightState;

typedef struct {
	bool isMainStreet;
	LightState intersectionState;
	int stateStep;

	int greenCountdown;
	SemaphoreColor center_west;
	int east_bound;
	SemaphoreColor center_east;
	int west_bound;
	SemaphoreColor main_north;
	int south_bound;
	SemaphoreColor main_south;
	int north_bound;

	int green_duration;
	int yellow_duration;
} MachineState;

void init(void) {
}

//--- PROGRAM ENTRY ---------------------------------------------------------------------------------------------------------------
bool run = true;

//typedef struct {
	int mainStreet_carCounter;
	int centerStreet_carCounter;
//} IntersectionState;

#define CENTER_STREET_MIN_QUEUE 2
#define MAIN_STREET_MIN_QUEUE 2

//int east_bound = 0, west_bound = 0, south_bound = 0, north_bound;

//const char *getSemaphoreGraphic(SemaphoreColor center_west, SemaphoreColor center_east, SemaphoreColor main_north, SemaphoreColor main_south) {
const char *getSemaphoreGraphic(MachineState *state) {
	static char graphic[1024];
	const char *semaphores[] = {
		_RESET_ "[OO" _GREEN_  "0" _RESET_   "]",
		_RESET_ "[O"  _YELLOW_ "0" _RESET_  "O]",
		_RESET_ "["   _RED_    "0" _RESET_ "OO]",
		_RESET_ "[OOO]"
	};
	int index = 0;

//--- North Main St.
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c     |\n", (state->north_bound > 5? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c     |\n", (state->north_bound > 4? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c M   |\n", (state->north_bound > 3? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c a   |\n", (state->north_bound > 2? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c i   |\n", (state->north_bound > 1? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c n   |\n", (state->north_bound > 0? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "---------  %s  ---------\n", semaphores[state->main_north]);

//--- East Center St.
	index += snprintf(graphic + index, sizeof(graphic) - index, "                  %c%c%c%c%c%c%c%c%c\n",
			(state->east_bound > 0? 'n': ' '), (state->east_bound > 1? 'n': ' '), (state->east_bound > 2? 'n': ' '),
			(state->east_bound > 3? 'n': ' '), (state->east_bound > 4? 'n': ' '), (state->east_bound > 5? 'n': ' '),
			(state->east_bound > 6? 'n': ' '), (state->east_bound > 7? 'n': ' '), (state->east_bound > 8? 'n': ' '));

	index += snprintf(graphic + index, sizeof(graphic) - index, "Center %s   %s Street\n", semaphores[state->center_west], semaphores[state->center_east]);

//--- West Center St.
	index += snprintf(graphic + index, sizeof(graphic) - index, "%c%c%c%c%c%c%c%c%c\n",
			(state->west_bound > 8? 'n': ' '), (state->west_bound > 7? 'n': ' '), (state->west_bound > 6? 'n': ' '),
			(state->west_bound > 5? 'n': ' '), (state->west_bound > 4? 'n': ' '), (state->west_bound > 3? 'n': ' '),
			(state->west_bound > 2? 'n': ' '), (state->west_bound > 1? 'n': ' '), (state->west_bound > 0? 'n': ' '));

//--- South Main St.
	index += snprintf(graphic + index, sizeof(graphic) - index, "---------  %s  ---------\n", semaphores[state->main_south]);
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   S %c |\n", (state->south_bound > 0? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   t %c |\n", (state->south_bound > 1? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   r %c |\n", (state->south_bound > 2? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   e %c |\n", (state->south_bound > 3? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   e %c |\n", (state->south_bound > 4? 'n': ' '));
	index += snprintf(graphic + index, sizeof(graphic) - index, "         |   t %c |\n", (state->south_bound > 5? 'n': ' '));

	return graphic;
}

#define DEFAULT_YELLOW_SECS 1
#define DEFAULT_GREEN_SECS (15 + DEFAULT_YELLOW_SECS)

void state_loop(MachineState *state) {
	while ( run ) {
		switch ( state->intersectionState ) {
			case e4WayStop_noTraffic:
				state->isMainStreet = ((state->stateStep % 2) == 0);
				if ( state->isMainStreet ) {
					fprintf(stderr, "Main Street's red light turns on. Center Street's red light turns off.\n");
					state->center_west = state->center_east = eOff;
					state->main_north = state->main_south = eRed;
					fprintf(stderr, "%s", getSemaphoreGraphic(state));
					if ( state->east_bound > 0 ) {
						state->east_bound--;
					}
					if ( state->west_bound > 0 ) {
						state->west_bound--;
					}
				}
				if ( !state->isMainStreet ) {
					fprintf(stderr, "Main Street's red light turns off. Center Street's red light turns on.\n");
					state->center_west = state->center_east = eRed;
					state->main_north = state->main_south = eOff;
					fprintf(stderr, "%s", getSemaphoreGraphic(state));
					if ( state->south_bound > 0 ) {
						state->south_bound--;
					}
					if ( state->north_bound > 0 ) {
						state->north_bound--;
					}
				}

				if ( mainStreet_carCounter > MAIN_STREET_MIN_QUEUE  ||  centerStreet_carCounter > CENTER_STREET_MIN_QUEUE ) {
					state->intersectionState = e4Way_to_NormalOperation;
				}
				break;

			case e4Way_to_NormalOperation:
				if ( state->isMainStreet ) {
					fprintf(stderr, "Main Street's red light turns off; green turns on. Center Street's red light turns on.\n");
					state->center_west = state->center_east = eGreen;
					state->main_north = state->main_south = eRed;
					fprintf(stderr, "%s", getSemaphoreGraphic(state));

				} else { // <-- !isMainStreet (or "isCenterStreet")
					fprintf(stderr, "Main Street's red light turns on. Center Street's red light turns off; green turns on.\n");
					state->center_west = state->center_east = eRed;
					state->main_north = state->main_south = eGreen;
					fprintf(stderr, "%s", getSemaphoreGraphic(state));
				}

				state->greenCountdown = DEFAULT_GREEN_SECS;
				state->intersectionState = eCountdown;
				break;

			case eCountdown:
				if ( state->greenCountdown > 1 ) {
					fprintf(stderr, ".");
				}
				state->greenCountdown--;
				if ( isBetween(0, state->greenCountdown, YELLOW_DURATION_DEFAULT) ) {
					if ( state->isMainStreet ) {
						state->center_west = state->center_east = eYellow;
						state->main_north = state->main_south = eRed;
						fprintf(stderr, "%s", getSemaphoreGraphic(state));

					} else {
						state->center_west = state->center_east = eRed;
						state->main_north = state->main_south = eYellow;
						fprintf(stderr, "%s", getSemaphoreGraphic(state));
					}

				} else if ( state->greenCountdown <= 0 ) {
					if ( state->isMainStreet ) {
						state->center_west = state->center_east = eRed;
						state->main_north = state->main_south = eGreen;
						fprintf(stderr, "%s", getSemaphoreGraphic(state));

					} else {
						state->center_west = state->center_east = eGreen;
						state->main_north = state->main_south = eRed;
						fprintf(stderr, "%s", getSemaphoreGraphic(state));
					}
					state->greenCountdown = DEFAULT_GREEN_SECS;
					state->isMainStreet = !state->isMainStreet;
				}
				break;

			case eNormalOperation://_lightTraffic:

				break;

			default:
				fprintf(stderr, "ERROR! Cannot get here!\n");
				abort();
		}
		sleep(1);
		state->stateStep++;

		if ( isBetween(10, random() % 100, 20) ) {
			state->south_bound++;
		}
		if ( isBetween(10, random() % 100, 20) ) {
			state->north_bound++;
		}
		if ( isBetween(40, random() % 100, 50) ) {
			state->east_bound++;
		}
		if ( isBetween(40, random() % 100, 50) ) {
			state->west_bound++;
		}
	}
}

//--- TOOLS -----------------------------------------------------------------------------------------------------------------------
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

void parse_args(const char *args[], MachineState *state) {
	if ( args != nullptr ) {
		while ( *args != nullptr ) {
			if ( str_startsWith(*args, "--red-delay=") ) {
				const char *seconds = *args + strlen("--red-delay=");
				state->green_duration = ( atoi(seconds)? : GREEN_DURATION_DEFAULT );

			} else if ( str_startsWith(*args, "--yellow-delay=") ) {
				const char *seconds = *args + strlen("--yellow-delay=");
				state->yellow_duration = ( atoi(seconds)? : YELLOW_DURATION_DEFAULT );

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

//---------------------------------------------------------------------------------------------------------------------------------
int main(int cnt, const char *args[]) {
	if ( args[1] != nullptr  &&  strcasecmp(args[1], "test") == 0 ) {
		int run_tests(void);
		return run_tests();

	} else {
		MachineState machineState = {
				.isMainStreet = false,
				.intersectionState = e4WayStop_noTraffic,
				.stateStep = 0,
				.greenCountdown = 0,

				.center_west = eRed,
				.east_bound = 0,
				.center_east = eRed,
				.west_bound = 0,
				.main_north = eRed,
				.south_bound = 0,
				.main_south = eRed,
				.north_bound = 0,

				.green_duration = DEFAULT_GREEN_SECS,
				.yellow_duration = DEFAULT_YELLOW_SECS
			};

		parse_args(++args, &machineState);
		init();
		state_loop(&machineState);

		return EXIT_SUCCESS;
	}
}

//=================================================================================================================================
//--- UNIT TESTS ------------------------------------------------------------------------------------------------------------------
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
		char nl = (strchr(expected_text, '\n') == nullptr? '\n': ' ');
		fprintf(stderr,
				"%s:%s[%d]: %s ≟ %s... "
					_RED_ "FAILED. Expected %c" _RESET_ "%s" _RED_ " but got %c" _RESET_ "%s" _RED_ "." _RESET_ "\n",
				file, function, linenum, expected_text, got_text, nl, expected, nl, got);
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

#define SEW_TEST_ASSERT(test) test_assert(__FILE__, __PRETTY_FUNCTION__, __LINE__, #test, test)
#define SEW_TEST_INT_EQUALS(expected, result) test_int_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (long)result, (long)expected)
#define SEW_TEST_STR_EQUALS(expected, result) test_str_equals(__FILE__, __PRETTY_FUNCTION__, __LINE__, #result, #expected, (const char*)result, (const char*)expected)

void test__parse_args(void) {
	MachineState state = {
			.intersectionState = eUnknown,
			.center_west = eRed,
			.east_bound = 0,
			.center_east = eRed,
			.west_bound = 0,
			.main_north = eRed,
			.south_bound = 0,
			.main_south = eRed,
			.north_bound = 0,

			.green_duration = 90,
			.yellow_duration = 5
		};

	{	const char *args[] = {nullptr};
		state.green_duration = -1;
		state.yellow_duration = -1;

		parse_args(args, &state);

		SEW_TEST_INT_EQUALS(state.green_duration, -1);
		SEW_TEST_INT_EQUALS(state.yellow_duration, -1);
		SEW_TEST_INT_EQUALS(state.intersectionState, eUnknown);
	}
	{
		const char *args[] = { "--preferred=center", "--red-delay=100", "--yellow-delay=200" };

		state.green_duration = -1;
		state.yellow_duration = -1;
		state.intersectionState = eUnknown;

		parse_args(args, &state);

		SEW_TEST_INT_EQUALS(state.green_duration, 100);
		SEW_TEST_INT_EQUALS(state.yellow_duration, 200);
		SEW_TEST_INT_EQUALS(state.intersectionState, eUnknown);
	}
	{
		const char *args[] = { "--preferred=main", "--red-delay=1", "--yellow-delay=2" };

		state.green_duration = -1;
		state.yellow_duration = -1;
		state.intersectionState = eUnknown;

		parse_args(args, &state);

		SEW_TEST_INT_EQUALS(state.green_duration, 1);
		SEW_TEST_INT_EQUALS(state.yellow_duration, 2);
		SEW_TEST_INT_EQUALS(state.intersectionState, eUnknown);
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

void test__isBetween(void) {
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

void test__getSemaphoreGraphic(void) {
	MachineState state = {
			.intersectionState = eUnknown,
			.center_west = eRed,
			.east_bound = 0,
			.center_east = eRed,
			.west_bound = 0,
			.main_north = eRed,
			.south_bound = 0,
			.main_south = eRed,
			.north_bound = 0
		};

	{//--- no cars
#define NORTH_BOUND_LANE_NONE	"         |       |\n" \
								"         |       |\n" \
								"         |   M   |\n" \
								"         |   a   |\n" \
								"         |   i   |\n" \
								"         |   n   |\n"

#define SOUTH_BOUND_LANE_NONE	"         |   S   |\n" \
								"         |   t   |\n" \
								"         |   r   |\n" \
								"         |   e   |\n" \
								"         |   e   |\n" \
								"         |   t   |\n"
		state.west_bound = state.east_bound = 0;
		state.north_bound = state.south_bound = 0;
		const char *red_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[" _RED_ "0" _RESET_ "OO]   " _RESET_ "[" _RED_ "0" _RESET_ "OO] Street\n"
				"         \n"
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.center_west = state.center_east = eRed;
		state.main_north = state.main_south = eRed;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), red_graphic);

		const char *yellow_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]   " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O] Street\n"
				"         \n"
				"---------  " _RESET_ "[O\x1B[93m0" _RESET_ "O]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.center_west = state.center_east = eYellow;
		state.main_north = state.main_south = eYellow;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), yellow_graphic);

		const char *green_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]   " _RESET_ "[OO" _GREEN_ "0" _RESET_ "] Street\n"
				"         \n"
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.center_west = state.center_east = eGreen;
		state.main_north = state.main_south = eGreen;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), green_graphic);

		const char *off_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[OOO]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[OOO]   " _RESET_ "[OOO] Street\n"
				"         \n"
				"---------  " _RESET_ "[OOO]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.center_west = state.center_east = eOff;
		state.main_north = state.main_south = eOff;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), off_graphic);
	}
	{//--- Some cars ------------------------------------------------------------------------------
#define NORTH_BOUND_LANE_SOME	"         |       |\n" \
								"         |       |\n" \
								"         |   M   |\n" \
								"         | n a   |\n" \
								"         | n i   |\n" \
								"         | n n   |\n"

#define SOUTH_BOUND_LANE_SOME	"         |   S n |\n" \
								"         |   t n |\n" \
								"         |   r n |\n" \
								"         |   e n |\n" \
								"         |   e   |\n" \
								"         |   t   |\n"

		state.west_bound = 1;
		state.east_bound = 2;
		state.north_bound = 3;
		state.south_bound = 4;
		const char *red_graphic =
				NORTH_BOUND_LANE_SOME
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				"                  nn       \n"
				"Center " _RESET_ "[" _RED_ "0" _RESET_ "OO]   " _RESET_ "[" _RED_ "0" _RESET_ "OO] Street\n"
				"        n\n"
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				SOUTH_BOUND_LANE_SOME
				;
		state.center_west = state.center_east = eRed;
		state.main_north = state.main_south = eRed;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), red_graphic);

		const char *yellow_graphic =
				NORTH_BOUND_LANE_SOME
				"---------  " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]  ---------\n"
				"                  nn       \n"
				"Center " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]   " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O] Street\n"
				"        n\n"
				"---------  " _RESET_ "[O\x1B[93m0" _RESET_ "O]  ---------\n"
				SOUTH_BOUND_LANE_SOME;
		state.center_west = state.center_east = eYellow;
		state.main_north = state.main_south = eYellow;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), yellow_graphic);

		const char *green_graphic =
				NORTH_BOUND_LANE_SOME
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				"                  nn       \n"
				"Center " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]   " _RESET_ "[OO" _GREEN_ "0" _RESET_ "] Street\n"
				"        n\n"
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				SOUTH_BOUND_LANE_SOME;
		state.center_west = state.center_east = eGreen;
		state.main_north = state.main_south = eGreen;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), green_graphic);

		const char *off_graphic =
				NORTH_BOUND_LANE_SOME
				"---------  " _RESET_ "[OOO]  ---------\n"
				"                  nn       \n"
				"Center " _RESET_ "[OOO]   " _RESET_ "[OOO] Street\n"
				"        n\n"
				"---------  " _RESET_ "[OOO]  ---------\n"
				SOUTH_BOUND_LANE_SOME;
		state.center_west = state.center_east = eOff;
		state.main_north = state.main_south = eOff;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), off_graphic);
	}
	{//--- Excess cars ----------------------------------------------------------------------------
#define NORTH_BOUND_LANE_EXCESS	"         | n     |\n" \
								"         | n     |\n" \
								"         | n M   |\n" \
								"         | n a   |\n" \
								"         | n i   |\n" \
								"         | n n   |\n"

#define SOUTH_BOUND_LANE_EXCESS	"         |   S n |\n" \
								"         |   t n |\n" \
								"         |   r n |\n" \
								"         |   e n |\n" \
								"         |   e n |\n" \
								"         |   t n |\n"

		state.west_bound = state.east_bound = 10;
		state.north_bound = state.south_bound = 10;
		const char *red_graphic =
				NORTH_BOUND_LANE_EXCESS
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				"                  nnnnnnnnn\n"
				"Center " _RESET_ "[" _RED_ "0" _RESET_ "OO]   " _RESET_ "[" _RED_ "0" _RESET_ "OO] Street\n"
				"nnnnnnnnn\n"
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				SOUTH_BOUND_LANE_EXCESS
				;
		state.center_west = state.center_east = eRed;
		state.main_north = state.main_south = eRed;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), red_graphic);

		const char *yellow_graphic =
				NORTH_BOUND_LANE_EXCESS
				"---------  " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]  ---------\n"
				"                  nnnnnnnnn\n"
				"Center " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]   " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O] Street\n"
				"nnnnnnnnn\n"
				"---------  " _RESET_ "[O\x1B[93m0" _RESET_ "O]  ---------\n"
				SOUTH_BOUND_LANE_EXCESS
				;
		state.center_west = state.center_east = eYellow;
		state.main_north = state.main_south = eYellow;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), yellow_graphic);

		const char *green_graphic =
				NORTH_BOUND_LANE_EXCESS
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				"                  nnnnnnnnn\n"
				"Center " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]   " _RESET_ "[OO" _GREEN_ "0" _RESET_ "] Street\n"
				"nnnnnnnnn\n"
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				SOUTH_BOUND_LANE_EXCESS
				;
		state.center_west = state.center_east = eGreen;
		state.main_north = state.main_south = eGreen;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), green_graphic);
		const char *off_graphic =

				NORTH_BOUND_LANE_EXCESS
				"---------  " _RESET_ "[OOO]  ---------\n"
				"                  nnnnnnnnn\n"
				"Center " _RESET_ "[OOO]   " _RESET_ "[OOO] Street\n"
				"nnnnnnnnn\n"
				"---------  " _RESET_ "[OOO]  ---------\n"
				SOUTH_BOUND_LANE_EXCESS
				;
		state.center_west = state.center_east = eOff;
		state.main_north = state.main_south = eOff;
		SEW_TEST_STR_EQUALS(getSemaphoreGraphic(&state), off_graphic);
	}
}

int run_tests(void) {
	test__parse_args();
	test__str_startsWith();
	test__isBetween();
	test__getSemaphoreGraphic();
	fprintf((failed > 0? stderr: stdout), "%d/%d tests failed\n", failed, failed + passed);
	return 0;
}
