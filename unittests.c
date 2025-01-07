/*
 * unittests.c
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */
#include <stdio.h>

#include "unittester.h"
#include "PassiveLogic_StreetSignal.h"

extern int passed, failed;

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



