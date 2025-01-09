/*
 * unittests.c
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */
#include <stdio.h>
#include <string.h>

#include "unittester.h"
#include "PassiveLogic_StreetSignal.h"
#include "PassiveLogic_Tools.h"

extern int passed, failed;

void test__parse_args(void) {
	MachineState state;
	MachineState_init(&state);

	{	const char *args[] = {nullptr};
//		state.green_duration = -1;
//		state.yellow_duration = -1;

		parse_args(args, &state);

//		TEST_INT_EQUALS(state.green_duration, -1);
//		TEST_INT_EQUALS(state.yellow_duration, -1);
		TEST_INT_EQUALS(state.intersectionState, e4WayStop);
	}
	{
		const char *args[] = { "--preferred=center", "--green-duration=100", "--yellow-delay=200", nullptr };

//		state.green_duration = -1;
//		state.yellow_duration = -1;
		state.intersectionState = eUnknown;

		parse_args(args, &state);

//		TEST_INT_EQUALS(state.green_duration, 100);
//		TEST_INT_EQUALS(state.yellow_duration, 200);
		TEST_INT_EQUALS(state.intersectionState, eUnknown);
	}
	{
		const char *args[] = { "--preferred=main", "--green-duration=1", "--yellow-delay=2", nullptr };

//		state.green_duration = -1;
//		state.yellow_duration = -1;
		state.intersectionState = eUnknown;

		parse_args(args, &state);

//		TEST_INT_EQUALS(state.green_duration, 1);
//		TEST_INT_EQUALS(state.yellow_duration, 2);
		TEST_INT_EQUALS(state.intersectionState, eUnknown);
	}
}

void test__str_startsWith(void) {
	TEST_ASSERT(!str_startsWith(nullptr, nullptr));
	TEST_ASSERT(!str_startsWith(nullptr, ""));
	TEST_ASSERT(str_startsWith("", nullptr));
	TEST_ASSERT(str_startsWith("", ""));
	TEST_ASSERT(str_startsWith("a", nullptr));
	TEST_ASSERT(str_startsWith("abc", ""));
	TEST_ASSERT(str_startsWith("abc", "a"));
	TEST_ASSERT(str_startsWith("abc", "ab"));
	TEST_ASSERT(!str_startsWith("abc", "ax"));
	TEST_ASSERT(!str_startsWith("abc", "bc"));
	TEST_ASSERT(!str_startsWith("abc", "x"));
}

void test__isBetween(void) {
	TEST_ASSERT(!isBetween(0,0,0));
	TEST_ASSERT(!isBetween(0,0,10));
	TEST_ASSERT(!isBetween(0,10,0));
	TEST_ASSERT(!isBetween(0,10,10));
	TEST_ASSERT(!isBetween(10,0,0));
	TEST_ASSERT(!isBetween(10,0,10));
	TEST_ASSERT(!isBetween(10,10,0));
	TEST_ASSERT(!isBetween(10,10,10));

	TEST_ASSERT(isBetween(0,5,10));
	TEST_ASSERT(isBetween(0,1,10));
	TEST_ASSERT(isBetween(0,9,10));
}

void test__getSemaphoreGraphic(void) {
	MachineState state;
	MachineState_init(&state);

	{//--- no cars
#define NORTH_BOUND_LANE_NONE	"===================================================\n" \
								"         |       |\n" \
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
		state.west_bound_counter = state.east_bound_counter = 0;
		state.north_bound_counter = state.south_bound_counter = 0;
		const char *red_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[" _RED_ "0" _RESET_ "OO]   " _RESET_ "[" _RED_ "0" _RESET_ "OO] Street\n"
				"                           \n"
				"---------  " _RESET_ "[" _RED_ "0" _RESET_ "OO]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.westbound = state.center_eastbound = eRed;
		state.main_northbound = state.main_southbound = eRed;

		TEST_STR_EQUALS(getTrafficSignalGraphic(&state), red_graphic);

		const char *yellow_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O]   " _RESET_ "[O" _YELLOW_ "0" _RESET_ "O] Street\n"
				"                           \n"
				"---------  " _RESET_ "[O\x1B[93m0" _RESET_ "O]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.westbound = state.center_eastbound = eYellow;
		state.main_northbound = state.main_southbound = eYellow;
		TEST_STR_EQUALS(getTrafficSignalGraphic(&state), yellow_graphic);

		const char *green_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]   " _RESET_ "[OO" _GREEN_ "0" _RESET_ "] Street\n"
				"                           \n"
				"---------  " _RESET_ "[OO" _GREEN_ "0" _RESET_ "]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.westbound = state.center_eastbound = eGreen;
		state.main_northbound = state.main_southbound = eGreen;
		TEST_STR_EQUALS(getTrafficSignalGraphic(&state), green_graphic);

		const char *off_graphic =
				NORTH_BOUND_LANE_NONE
				"---------  " _RESET_ "[OOO]  ---------\n"
				"                           \n"
				"Center " _RESET_ "[OOO]   " _RESET_ "[OOO] Street\n"
				"                           \n"
				"---------  " _RESET_ "[OOO]  ---------\n"
				SOUTH_BOUND_LANE_NONE;
		state.westbound = state.center_eastbound = eOff;
		state.main_northbound = state.main_southbound = eOff;
		TEST_STR_EQUALS(getTrafficSignalGraphic(&state), off_graphic);
	}
}

char *safe_strcpy(char *dst, const char *src, size_t len) {
	strncpy(dst, src, len);
	dst[len - 1] = 0;
	return dst;
}

void test__MoveLane(void) {
	char lane[20], queue[20];

//---Empty---------------------
	{
	//--- isForwards, hasGreenLight
		safe_strcpy(queue, "UVWXYZ", sizeof(queue));
		safe_strcpy(lane, "0123456789A", sizeof(lane));
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWXY");
		TEST_STR_EQUALS(lane, "Z0123456789");

	//--- isForwards, !hasGreenLight
		safe_strcpy(queue, "UVWXYZ", sizeof(queue));
		safe_strcpy(lane, "0123456789A", sizeof(lane));
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWXY");
		TEST_STR_EQUALS(lane, " 0123456789");

	//--- !isForwards, hasGreenLight
		safe_strcpy(queue, "ZYXWVU", sizeof(queue));
		safe_strcpy(lane, "A9876543210", sizeof(lane));
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "YXWVU ");
		TEST_STR_EQUALS(lane, "9876543210Z");

	//--- !isForwards, !hasGreenLight
		safe_strcpy(queue, "ZYXWVU", sizeof(queue));
		safe_strcpy(lane, "A9876543210", sizeof(lane));
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "YXWVU ");
		TEST_STR_EQUALS(lane, "9876543210 ");
	}
//---1 vehicle in exit lane----
	{
	//--- isForwards, hasGreenLight
		safe_strcpy(queue, "UVWXYZ", sizeof(queue));
		safe_strcpy(lane, ">0123456789", sizeof(lane));
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWXY");
		TEST_STR_EQUALS(lane, "Z>012345678");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "  UVWX");
		TEST_STR_EQUALS(lane, "YZ>01234567");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   UVW");
		TEST_STR_EQUALS(lane, "XYZ>0123456");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "    UV");
		TEST_STR_EQUALS(lane, "WXYZ>012345");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "     U");
		TEST_STR_EQUALS(lane, "VWXYZ>01234");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "UVWXYZ>0123");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, " UVWXYZ>012");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "  UVWXYZ>01");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "   UVWXYZ>0");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "    UVWXYZ>");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "     UVWXYZ");

	//--- isForwards, !hasGreenLight
		safe_strcpy(queue, "UVWXYZ", sizeof(queue));
		safe_strcpy(lane, ">0123456789", sizeof(lane));
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWXY");
		TEST_STR_EQUALS(lane, " >012345678");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "  UVWX");
		TEST_STR_EQUALS(lane, "  >01234567");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   UVW");
		TEST_STR_EQUALS(lane, "   >0123456");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "    UV");
		TEST_STR_EQUALS(lane, "    >012345");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "     U");
		TEST_STR_EQUALS(lane, "     >01234");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "      >0123");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "       >012");

	//--- !isForwards, hasGreenLight
		safe_strcpy(queue, "ZYXWVU", sizeof(queue));
		safe_strcpy(lane, "9876543210<", sizeof(lane));
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "YXWVU ");
		TEST_STR_EQUALS(lane, "876543210<Z");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "XWVU  ");
		TEST_STR_EQUALS(lane, "76543210<ZY");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "WVU   ");
		TEST_STR_EQUALS(lane, "6543210<ZYX");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "VU    ");
		TEST_STR_EQUALS(lane, "543210<ZYXW");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "U     ");
		TEST_STR_EQUALS(lane, "43210<ZYXWV");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "3210<ZYXWVU");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "210<ZYXWVU ");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "10<ZYXWVU  ");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "0<ZYXWVU   ");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "<ZYXWVU    ");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "ZYXWVU     ");

	//--- !isForwards, !hasGreenLight
		safe_strcpy(queue, "ZYXWVU", sizeof(queue));
		safe_strcpy(lane, "9876543210<", sizeof(lane));
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "YXWVU ");
		TEST_STR_EQUALS(lane, "876543210< ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "XWVU  ");
		TEST_STR_EQUALS(lane, "76543210<  ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "WVU   ");
		TEST_STR_EQUALS(lane, "6543210<   ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "VU    ");
		TEST_STR_EQUALS(lane, "543210<    ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "U     ");
		TEST_STR_EQUALS(lane, "43210<     ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "3210<      ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "210<       ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "10<        ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "0<         ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "<          ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "           ");
	}
//---2 vehicles in exit lane; 1 at stoplight -----------------------------
	{
	//--- isForwards, hasGreenLight
		safe_strcpy(queue, "UVWXY>", sizeof(queue));
		safe_strcpy(lane, ">01>3456789", sizeof(lane));
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWXY");
		TEST_STR_EQUALS(lane, ">>01>345678");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "  UVWX");
		TEST_STR_EQUALS(lane, "Y>>01>34567");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   UVW");
		TEST_STR_EQUALS(lane, "XY>>01>3456");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "    UV");
		TEST_STR_EQUALS(lane, "WXY>>01>345");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "     U");
		TEST_STR_EQUALS(lane, "VWXY>>01>34");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "UVWXY>>01>3");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, " UVWXY>>01>");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "  UVWXY>>01");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "   UVWXY>>0");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "    UVWXY>>");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "     UVWXY>");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, "      UVWXY");
	//NOTE: the *exhaustive* shift test is complete.

	//--- isForwards, !hasGreenLight
		safe_strcpy(queue, "UVWXY>", sizeof(queue));
		safe_strcpy(lane, ">01>3456789", sizeof(lane));
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWX>");
		TEST_STR_EQUALS(lane, " >01>345678");

	//--- !isForwards, hasGreenLight
		safe_strcpy(queue, "<YXWVU", sizeof(queue));
		safe_strcpy(lane, "9876543<10<", sizeof(lane));
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "YXWVU ");
		TEST_STR_EQUALS(lane, "876543<10<<");

	//--- !isForwards, !hasGreenLight
		safe_strcpy(queue, "<YXWVU", sizeof(queue));
		safe_strcpy(lane, "9876543<10<", sizeof(lane));
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "<XWVU ");
		TEST_STR_EQUALS(lane, "876543<10< ");
	}
//---4 vehicles in exit lane; 2 at stoplight -----------------------------
	{
	//--- isForwards, hasGreenLight
		safe_strcpy(queue, "UVWX>>", sizeof(queue));
		safe_strcpy(lane, ">1>>45>789", sizeof(lane));
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVWX>");
		TEST_STR_EQUALS(lane, ">>1>>45>78");

	//--- isForwards, !hasGreenLight
		safe_strcpy(queue, "UVWX>>", sizeof(queue));
		safe_strcpy(lane, ">01>>45>789", sizeof(lane));
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, " UVW>>");
		TEST_STR_EQUALS(lane, " >01>>45>78");

	//--- !isForwards, hasGreenLight
		safe_strcpy(queue, "<<XWVU", sizeof(queue));
		safe_strcpy(lane, "987<54<<10<", sizeof(lane));
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "<XWVU ");
		TEST_STR_EQUALS(lane, "87<54<<10<<");

	//--- !isForwards, !hasGreenLight
		safe_strcpy(queue, "<<XWVU", sizeof(queue));
		safe_strcpy(lane, "987<54<<10<", sizeof(lane));
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "<<WVU ");
		TEST_STR_EQUALS(lane, "87<54<<10< ");
	}
//---3 vehicles in exit lane & 1 drops off; 2 at stoplight & 1 approach -----------------------------
	{
	//--- isForwards, hasGreenLight
		safe_strcpy(queue, ">VWX>>", sizeof(queue));
		safe_strcpy(lane, ">012345>78>", sizeof(lane));
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "  >WX>");
		TEST_STR_EQUALS(lane, ">>012345>78");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "    >X");
		TEST_STR_EQUALS(lane, ">>>012345>7");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "     >");
		TEST_STR_EQUALS(lane, "X>>>012345>");
		MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "      ");
		TEST_STR_EQUALS(lane, ">X>>>012345");
/*FIXME*/MoveLane(true, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "      ");
/*FIXME*/TEST_STR_EQUALS(lane, " >X>>>01234");

	//--- isForwards, !hasGreenLight
		safe_strcpy(queue, ">VWX>>", sizeof(queue));
		safe_strcpy(lane, ">012345>78>", sizeof(lane));
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, " >WX>>");
/*FIXME*/TEST_STR_EQUALS(lane, " >012345>78");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "  >X>>");
		TEST_STR_EQUALS(lane, "  >012345>7");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   >>>");
		TEST_STR_EQUALS(lane, "   >012345>");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   >>>");
		TEST_STR_EQUALS(lane, "    >012345");
		MoveLane(false, '>', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "   >>>");
		TEST_STR_EQUALS(lane, "     >01234");

	//--- !isForwards, hasGreenLight
		safe_strcpy(queue, "<<XWV<", sizeof(queue));
		safe_strcpy(lane, "<87<543210<", sizeof(lane));
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "<XW<  ");
/*FIXME*/TEST_STR_EQUALS(lane, "87<543210<<");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "X<    ");
/*FIXME*/TEST_STR_EQUALS(lane, "7<543210<<<");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "<     ");
		TEST_STR_EQUALS(lane, "<543210<<<X");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "      ");
/*FIXME*/TEST_STR_EQUALS(lane, "543210<<<X<");
		MoveLane(true, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "      ");
/*FIXME*/TEST_STR_EQUALS(lane, "43210<<<X< ");

	//--- !isForwards, !hasGreenLight
		safe_strcpy(queue, "<<XWV<", sizeof(queue));
		safe_strcpy(lane, "<87<543210<", sizeof(lane));
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "<<XW< ");
		TEST_STR_EQUALS(lane, "87<543210< ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
/*FIXME*/TEST_STR_EQUALS(queue, "<<X<  ");
		TEST_STR_EQUALS(lane, "7<543210<  ");
		MoveLane(false, '<', lane, strlen(lane), queue, strlen(queue));
		TEST_STR_EQUALS(queue, "<<<   ");
		TEST_STR_EQUALS(lane, "<543210<   ");
	}
}

int run_tests(void) {
	test__parse_args();
	test__str_startsWith();
	test__isBetween();
	test__getSemaphoreGraphic();
	test__MoveLane();
	fprintf((failed > 0? stderr: stdout), "%d/%d tests failed\n", failed, failed + passed);
	return 0;
}

