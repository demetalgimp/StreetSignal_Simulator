//tabstop=4
/*
 ============================================================================
 Name        : PassiveLogic_StreetSignal.c
 Author      : Sean Walton
 Version     : 0.1
 Copyright   : Copyright (c) 2025
 Description : Street Signal simulation
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "PassiveLogic_StreetSignal.h"

#define DEFAULT_YELLOW_SECS 2
#define DEFAULT_GREEN_SECS (10 + DEFAULT_YELLOW_SECS)

/*
 * Street signal simulation
 * 		Normal operation is simply running from the command line.
 * 		Running the unit tests requires a command line parameter 'test'.
 *
 * Notes:
 * 		* The simulation starts off with a four-way stop. Why? I had more plans for the
 * 		simulation than time permitted.
 * 		* I left out left/right turns.
 * 		* There's a glitch or two in the movement of sprites but the diagnostic panel
 * 		(the output above the ascii graphic) shows correct behavior.
 *		* Any unit tests that "fail" are cosmetic, no more. It's very difficult to test
 *		graphic output.
 *		* There's a lot that could be optimized especially the ascii graphic. I really
 *		would like to have "objectify" the data with methods that approximate OOP.
 */
//void TrafficLane_init(TrafficLane *lane, char symbol, bool is_reversed) {
//	memset(lane, 0, sizeof(*lane));
//	memset(lane->Lane, ' ', sizeof(lane->Lane));
//	lane->Lane[sizeof(lane->Lane) - 1] = 0;
//	memset(lane->Queue, ' ', sizeof(lane->Queue));
//	lane->Queue[sizeof(lane->Queue) - 1] = 0;
//	lane->Direction = is_reversed;
//	lane->Symbol = symbol;
//	lane->color = eOff;
//}

void MachineState_init(MachineState *state) {
	state->intersectionState = e4WayStop;
	state->stateStep = 0;
	state->greenCountdown = 0;

	state->westbound = eRed;
	state->eastbound = eRed;
	state->northbound = eRed;
	state->southbound = eRed;

//	init_TrafficLane(&state->east_bound, VEHICLE_SYMBOL__EAST_BOUND, false);
//	init_TrafficLane(&state->west_bound, VEHICLE_SYMBOL__WEST_BOUND, true);
//	init_TrafficLane(&state->north_bound, VEHICLE_SYMBOL__NORTH_BOUND, false);
//	init_TrafficLane(&state->south_bound, VEHICLE_SYMBOL__SOUTH_BOUND, true);
	memset(state->SouthBound_Queue, ' ', sizeof(state->SouthBound_Queue));
	memset(state->SouthBound_Lane, ' ', sizeof(state->SouthBound_Lane));
	memset(state->NorthBound_Queue, ' ', sizeof(state->NorthBound_Queue));
	memset(state->NorthBound_Lane, ' ', sizeof(state->NorthBound_Lane));
	memset(state->EastBound_Queue, ' ', sizeof(state->EastBound_Queue));
	memset(state->EastBound_Lane, ' ', sizeof(state->EastBound_Lane));
	memset(state->WestBound_Queue, ' ', sizeof(state->WestBound_Queue));
	memset(state->WestBound_Lane, ' ', sizeof(state->WestBound_Lane));
	state->SouthBound_Queue[sizeof(state->SouthBound_Queue) - 1] = 0;
	state->SouthBound_Lane[sizeof(state->SouthBound_Lane) - 1] = 0;
	state->NorthBound_Queue[sizeof(state->NorthBound_Queue) - 1] = 0;
	state->NorthBound_Lane[sizeof(state->NorthBound_Lane) - 1] = 0;
	state->EastBound_Queue[sizeof(state->EastBound_Queue) - 1] = 0;
	state->EastBound_Lane[sizeof(state->EastBound_Lane) - 1] = 0;
	state->WestBound_Queue[sizeof(state->WestBound_Queue) - 1] = 0;
	state->WestBound_Lane[sizeof(state->WestBound_Lane) - 1] = 0;
}

//--- PROGRAM ENTRY ---------------------------------------------------------------------------------------------------------------
bool run = true;

#define EAST_WEST_MIN_QUEUE 2
#define NORTH_SOUTH_MIN_QUEUE 2

const char *toAnsiColor(SemaphoreColor color) {
	switch ( color ) {
		case eRed: return _RED_;
		case eYellow: return _YELLOW_;
		case eGreen: return _GREEN_;
		default: return _RESET_;
	}
}

const char *drawTrafficSignalGraphic(MachineState *state) {
	static char graphic[1024];
	static const char *semaphores[] = {
			_RESET_ "[OOO]",
			_RESET_ "[OO" _GREEN_  "0" _RESET_   "]",
			_RESET_ "[O"  _YELLOW_ "0" _RESET_  "O]",
			_RESET_ "["   _RED_    "0" _RESET_ "OO]"
		};

	printf("%sNorthbound" _RESET_ ": '%s':'%s'\n", toAnsiColor(state->northbound), state->NorthBound_Lane, state->NorthBound_Queue);
	printf("%sSouthbound" _RESET_ ": '%s':'%s'\n", toAnsiColor(state->southbound), state->SouthBound_Queue, state->SouthBound_Lane);
	printf("%s Eastbound" _RESET_ ": '%s':'%s'\n", toAnsiColor(state->eastbound), state->EastBound_Queue, state->EastBound_Lane);
	printf("%s Westbound" _RESET_ ": '%s':'%s'\n", toAnsiColor(state->westbound), state->WestBound_Lane, state->WestBound_Queue);

	memset(graphic, 0, sizeof(graphic));
	int index = 0;
	index += snprintf(graphic + index, sizeof(graphic) - index, "===================================================\n");
//--- North-bound traffic
	for ( int i = 0; i < STATE_TRAFFIC_LANE_SIZE - 1; i++ ) {
		static const char Label[STATE_TRAFFIC_LANE_SIZE] = "  Main"; // <-- I'm doing this so that I can reuse the Label variable
		char c_queue = (state->SouthBound_Queue[i]?: '*');	// <-- There's a defect somewhere that inserts a '\0'. This is a patch. I haven't the problem recently.
		char c_lane = (state->NorthBound_Lane[i]?: '*');	// (ditto)
		index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c %c %c |\n", c_queue, Label[i], c_lane);
	}
	index += snprintf(graphic + index, sizeof(graphic) - index, "---------  %s  ---------\n", semaphores[state->northbound]);

//--- West-bound traffic
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_lane = (state->WestBound_Lane[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_lane);
	}
	index += snprintf(graphic + index, sizeof(graphic) - index, "         ");
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_queue = (state->WestBound_Queue[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_queue);
	}
//--- Label & traffic light
	index += snprintf(graphic + index, sizeof(graphic) - index, "\nCenter %s   %s Street\n", semaphores[state->westbound], semaphores[state->eastbound]);

//--- East-bound traffic
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_queue = (state->EastBound_Queue[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_queue);
	}
	index += snprintf(graphic + index, sizeof(graphic) - index, "         ");
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_lane = (state->EastBound_Lane[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_lane);
	}

//--- South-bound traffic
	index += snprintf(graphic + index, sizeof(graphic) - index, "\n---------  %s  ---------\n", semaphores[state->southbound]);
	for ( int i = 0; i < STATE_TRAFFIC_LANE_SIZE - 1; i++ ) {
		static const char Label[STATE_TRAFFIC_LANE_SIZE] = "Street"; // <-- I'm doing this so that I can reuse the Label variable
		char c_queue = (state->NorthBound_Queue[i]?: '*');	// (ditto)
		char c_lane = (state->SouthBound_Lane[i]?: '*');	// (ditto)
		index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c %c %c |\n", c_lane, Label[i], c_queue);
	}

	return graphic;
}

void MoveLane(bool hasGreenLight, char vehicle_symbol, char *lane, int lane_size, char *queue, int queue_size) {
	bool isForwards = (vehicle_symbol == VEHICLE_SYMBOL__EAST_BOUND  ||  vehicle_symbol == VEHICLE_SYMBOL__SOUTH_BOUND);
	if ( isForwards ) { // [lane <-- ][queue <-- ]
		for ( int i = lane_size - 1; i > 0; i-- ) {
			lane[i] = lane[i - 1];
			lane[i - 1] = ' ';
		}
		if ( hasGreenLight ) {
			lane[0] = queue[queue_size - 1];
			for ( int i = queue_size - 1; i > 0; i-- ) {
				queue[i] = queue[i - 1];
				queue[i - 1] = ' ';
			}

		} else {
			lane[0] = ' ';
			for ( int i = queue_size - 1; i > 0; i-- ) {
				if ( queue[i] != vehicle_symbol ) {
					queue[i] = queue[i - 1];
					queue[i - 1] = ' '; // <-- this is necessary for the next iteration; don't optimize out.
				}
			}
		}
		queue[0] = ' ';

	} else { // "is backwards" [queue --> ][lane --> ]
		for ( int i = 0; i < lane_size - 1; i++ ) {
			lane[i] = lane[i + 1];
		}
		if ( hasGreenLight ) {
			lane[lane_size - 1] = queue[0];
			for ( int i = 0; i < queue_size - 1; i++ ) {
				queue[i] = queue[i + 1];
			}
			queue[queue_size - 1] = ' ';

		} else { // <-- red light, bunch up
			lane[lane_size - 1] = ' ';
			for ( int i = 0; i < queue_size - 1; i++ ) {
				if ( queue[i] != vehicle_symbol ) {
					queue[i] = queue[i + 1];
					queue[i + 1] = ' '; // <-- this is necessary for the next iteration; don't optimize out.
				}
			}
		}
		lane[lane_size] = 0;	// <-- /*patch*/ in case array boundary gets corrupted
		queue[queue_size] = 0;	// <-- /*patch*/ in case array boundary gets corrupted
	}

}

void MoveTraffic(MachineState *state) {
	MoveLane((state->northbound == eGreen), VEHICLE_SYMBOL__NORTH_BOUND, state->NorthBound_Lane, STATE_TRAFFIC_LANE_SIZE, state->NorthBound_Queue, STATE_TRAFFIC_LANE_SIZE);
	MoveLane((state->southbound == eGreen), VEHICLE_SYMBOL__SOUTH_BOUND, state->SouthBound_Lane, STATE_TRAFFIC_LANE_SIZE, state->SouthBound_Queue, STATE_TRAFFIC_LANE_SIZE);
	MoveLane((state->eastbound == eGreen), VEHICLE_SYMBOL__EAST_BOUND, state->EastBound_Lane, CENTER_TRAFFIC_LANE_SIZE, state->EastBound_Queue, CENTER_TRAFFIC_LANE_SIZE);
	MoveLane((state->westbound == eGreen), VEHICLE_SYMBOL__WEST_BOUND, state->WestBound_Lane, CENTER_TRAFFIC_LANE_SIZE, state->WestBound_Queue, CENTER_TRAFFIC_LANE_SIZE);
}

void LowTraffic_4WayStop(MachineState *state) {
	bool isMainStreet = ((state->stateStep % 2) == 0);
	if ( isMainStreet ) {
		printf("Main Street's red light turns on. Center Street's red light turns off.\n");
		state->westbound = state->eastbound = eOff;
		state->northbound = state->southbound = eRed;
	}

	if ( !isMainStreet ) {
		printf("Main Street's red light turns off. Center Street's red light turns on.\n");
		state->westbound = state->eastbound = eRed;
		state->northbound = state->southbound = eOff;
	}

	MoveTraffic(state);

	printf("%s", drawTrafficSignalGraphic(state));

	if ( strcnt(state->SouthBound_Queue, VEHICLE_SYMBOL__SOUTH_BOUND) > NORTH_SOUTH_MIN_QUEUE
			||  strcnt(state->NorthBound_Queue, VEHICLE_SYMBOL__NORTH_BOUND) > NORTH_SOUTH_MIN_QUEUE
			||  strcnt(state->EastBound_Queue, VEHICLE_SYMBOL__EAST_BOUND) > EAST_WEST_MIN_QUEUE
			||  strcnt(state->WestBound_Queue, VEHICLE_SYMBOL__WEST_BOUND) > EAST_WEST_MIN_QUEUE ) {
		state->intersectionState = e4WayStop_to_NormalOperation;
	}
}

void IncreasedTraffic_4WayStop_to_NormalOperations(MachineState *state) {
	if ( state->northbound == eGreen  &&  state->southbound == eGreen ) {
		printf("Main Street's red light turns off; green turns on. Center Street's red light turns on.\n");
		state->westbound = state->eastbound = eGreen;
		state->northbound = state->southbound = eRed;

	} else { //--- east- & west-bound lanes are open
		printf("Main Street's red light turns on. Center Street's red light turns off; green turns on.\n");
		state->westbound = state->eastbound = eRed;
		state->northbound = state->southbound = eGreen;
	}

	state->greenCountdown = DEFAULT_GREEN_SECS;
	state->intersectionState = eNormalOperation;
	printf("%s", drawTrafficSignalGraphic(state));
}

void NormalTraffic_GreenLightCountdown(MachineState *state) {
	state->greenCountdown--;

//--- Transition to "red"
	if ( state->greenCountdown <= 0 ) {
			if ( state->northbound == eYellow  &&  state->southbound == eYellow ) {
				state->westbound = state->eastbound = eGreen;
				state->northbound = state->southbound = eRed;

			} else { // <-- Center Street
				state->westbound = state->eastbound = eRed;
				state->northbound = state->southbound = eGreen;
			}
			state->greenCountdown = DEFAULT_GREEN_SECS;

//--- Transition "yellow" (thru-moving traffic)
	} else if ( isBetween(0, state->greenCountdown, DEFAULT_YELLOW_SECS) ) {
		if ( state->northbound != eRed  &&  state->southbound != eRed ) {
			state->northbound = state->southbound = eYellow;

		} else { // <-- Center Street
			state->westbound = state->eastbound = eYellow;
		}

	} else {//--- Operate under "green" state (thru-moving traffic)
		MoveTraffic(state);
	}

	printf("%s", drawTrafficSignalGraphic(state));
}

void state_loop(MachineState *state) {
	while ( run ) {
		switch ( state->intersectionState ) {
			case e4WayStop:
				LowTraffic_4WayStop(state);
				break;

			case e4WayStop_to_NormalOperation:
				IncreasedTraffic_4WayStop_to_NormalOperations(state);
				break;

			case eNormalOperation:
				NormalTraffic_GreenLightCountdown(state);
				break;

			default:
				printf("ERROR! Cannot get here!\n");
				abort();
		}

		sleep(1);
		state->stateStep++;

		if ( isBetween(0, random() % 100, 25) ) {
			state->SouthBound_Queue[0] = VEHICLE_SYMBOL__SOUTH_BOUND;
		}
		if ( isBetween(0, random() % 100, 25) ) {
			state->NorthBound_Queue[sizeof(state->NorthBound_Queue) - 2] = VEHICLE_SYMBOL__NORTH_BOUND;
			state->NorthBound_Queue[0] = VEHICLE_SYMBOL__NORTH_BOUND;
		}
		if ( isBetween(0, random() % 100, 25) ) {
			state->EastBound_Queue[0] = VEHICLE_SYMBOL__EAST_BOUND;
		}
		if ( isBetween(0, random() % 100, 25) ) {
			state->WestBound_Queue[sizeof(state->WestBound_Queue) - 2] = VEHICLE_SYMBOL__WEST_BOUND;
		}
	}
}

void parse_args(const char *args[], MachineState *state) {
	if ( args != nullptr ) {
		while ( *args != nullptr ) {
//			if ( str_startsWith(*args, "--green-delay=") ) {
//				const char *seconds = *args + strlen("--green-delay=");
//				state->green_duration = ( atoi(seconds)? : GREEN_DURATION_DEFAULT );
//
//			} else if ( str_startsWith(*args, "--yellow-delay=") ) {
//				const char *seconds = *args + strlen("--yellow-delay=");
//				state->yellow_duration = ( atoi(seconds)? : YELLOW_DURATION_DEFAULT );
//			}
			args++;
		}

	} else {
		printf("Command line args are NULL. Not possible!\n");
		abort();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
int main(int cnt, const char *args[]) {
	if ( args[1] != nullptr  &&  strcasecmp(args[1], "test") == 0 ) {
		int run_tests(void);
		return run_tests();

	} else {
		MachineState machineState;
		memset(&machineState, 0, sizeof(machineState));

		parse_args(++args, &machineState);
		MachineState_init(&machineState);
		state_loop(&machineState);

		return EXIT_SUCCESS;
	}
}
