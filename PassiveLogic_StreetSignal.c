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

#include "PassiveLogic_StreetSignal.h"

#define GREEN_DURATION_DEFAULT 90
#define YELLOW_DURATION_DEFAULT 3
#define DEFAULT_YELLOW_SECS 1
#define DEFAULT_GREEN_SECS (15 + DEFAULT_YELLOW_SECS)

//const char VehicleSymbols[] = {VEHICLE_SYMBOL__EAST_BOUND, VEHICLE_SYMBOL__WEST_BOUND, VEHICLE_SYMBOL__SOUTH_BOUND, VEHICLE_SYMBOL__NORTH_BOUND, 0};

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
void TrafficLane_init(TrafficLane *lane, char symbol, bool is_reversed) {
	memset(lane, 0, sizeof(*lane));
	memset(lane->Lane, ' ', sizeof(lane->Lane));
	lane->Lane[sizeof(lane->Lane) - 1] = 0;
	memset(lane->Queue, ' ', sizeof(lane->Queue));
	lane->Queue[sizeof(lane->Queue) - 1] = 0;
	lane->Direction = is_reversed;
	lane->Symbol = symbol;
	lane->color = eOff;
}

void TrafficLane_addVehicle(TrafficLane *lane) {
//TODO
}
void TrafficLane_rollVehicles(TrafficLane *lane) {
//TODO
}

void MachineState_init(MachineState *state) {
	state->intersectionState = e4WayStop;
	state->stateStep = 0;
	state->greenCountdown = 0;

	state->westbound = eRed;
	state->east_bound_counter = 0;
	state->center_eastbound = eRed;
	state->west_bound_counter = 0;
	state->main_northbound = eRed;
	state->south_bound_counter = 0;
	state->main_southbound = eRed;
	state->north_bound_counter = 0;

//	state->green_duration = DEFAULT_GREEN_SECS;
//	state->yellow_duration = DEFAULT_YELLOW_SECS;
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

#define CENTER_STREET_MIN_QUEUE 2
#define MAIN_STREET_MIN_QUEUE 2

const char *getTrafficSignalGraphic(MachineState *state) {
	static char graphic[1024];
	static const char *semaphores[] = {
			_RESET_ "[OOO]",
			_RESET_ "[OO" _GREEN_  "0" _RESET_   "]",
			_RESET_ "[O"  _YELLOW_ "0" _RESET_  "O]",
			_RESET_ "["   _RED_    "0" _RESET_ "OO]"
		};

	printf("Northbound: '%s':'%s'\n", state->NorthBound_Queue, state->NorthBound_Lane);
	printf("Southbound: '%s':'%s'\n", state->SouthBound_Queue, state->SouthBound_Lane);
	printf("Eastbound: '%s':'%s'\n", state->EastBound_Queue, state->EastBound_Lane);
	printf("Westbound: '%s':'%s'\n", state->WestBound_Queue, state->WestBound_Lane);
	memset(graphic, 0, sizeof(graphic));
	int index = 0;
	index += snprintf(graphic + index, sizeof(graphic) - index, "===================================================\n");
//--- North Main St.
	for ( int i = 0; i < STATE_TRAFFIC_LANE_SIZE - 1; i++ ) {
		static const char Label[STATE_TRAFFIC_LANE_SIZE] = "  Main"; // <-- I'm doing this so that I can reuse the Label variable
		char c_lane = (state->SouthBound_Lane[STATE_TRAFFIC_LANE_SIZE - i - 1]?: '*');
		char c_queue = (state->NorthBound_Queue[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c %c %c |\n", c_lane, Label[i], c_queue);
	}
	index += snprintf(graphic + index, sizeof(graphic) - index, "---------  %s  ---------\n", semaphores[state->main_northbound]);

//FIXME--- Need to make sure that the strings are terminated. *PATCH*
//	state->CenterStreet_WestBound_Lane[CENTER_TRAFFIC_LANE_SIZE - 1] = 0;
//	state->CenterStreet_WestBound_Queue[CENTER_TRAFFIC_LANE_SIZE - 1] = 0;
//	state->CenterStreet_EastBound_Lane[CENTER_TRAFFIC_LANE_SIZE - 1] = 0;
//	state->CenterStreet_EastBound_Queue[CENTER_TRAFFIC_LANE_SIZE - 1] = 0;

//--- East Center St.
//	index += snprintf(graphic + index, sizeof(graphic) - index, "%s         %s\n", state->CenterStreet_WestBound_Lane, state->CenterStreet_WestBound_Queue);
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
	index += snprintf(graphic + index, sizeof(graphic) - index, "\nCenter %s   %s Street\n", semaphores[state->westbound], semaphores[state->center_eastbound]);

//--- West Center St.
//	index += snprintf(graphic + index, sizeof(graphic) - index, "%s         %s\n", state->CenterStreet_EastBound_Queue, state->CenterStreet_EastBound_Lane);
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_queue = (state->EastBound_Queue[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_queue);
	}
	index += snprintf(graphic + index, sizeof(graphic) - index, "         ");
	for ( int i = 0; i < CENTER_TRAFFIC_LANE_SIZE - 1; i++ ) {
		char c_lane = (state->EastBound_Lane[i]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "%c", c_lane);
	}

//--- South Main St.
	index += snprintf(graphic + index, sizeof(graphic) - index, "\n---------  %s  ---------\n", semaphores[state->main_southbound]);
	for ( int i = 0; i < STATE_TRAFFIC_LANE_SIZE - 1; i++ ) {
		static const char Label[STATE_TRAFFIC_LANE_SIZE] = "Street"; // <-- I'm doing this so that I can reuse the Label variable
		char c_lane = (state->NorthBound_Lane[i]?: '*');
		char c_queue = (state->SouthBound_Queue[STATE_TRAFFIC_LANE_SIZE - i - 1]?: '*');
		index += snprintf(graphic + index, sizeof(graphic) - index, "         | %c %c %c |\n", c_queue, Label[i], c_lane);
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
//				if ( strchr(VehicleSymbols, queue[i]) == nullptr ) {
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
//				if ( strchr(VehicleSymbols, queue[i]) == nullptr ) {
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
	MoveLane((state->main_northbound == eGreen), VEHICLE_SYMBOL__NORTH_BOUND, state->NorthBound_Lane, STATE_TRAFFIC_LANE_SIZE, state->NorthBound_Queue, STATE_TRAFFIC_LANE_SIZE);
	MoveLane((state->main_southbound == eGreen), VEHICLE_SYMBOL__SOUTH_BOUND, state->SouthBound_Lane, STATE_TRAFFIC_LANE_SIZE, state->SouthBound_Queue, STATE_TRAFFIC_LANE_SIZE);
	MoveLane((state->center_eastbound == eGreen), VEHICLE_SYMBOL__EAST_BOUND, state->EastBound_Lane, CENTER_TRAFFIC_LANE_SIZE, state->EastBound_Queue, CENTER_TRAFFIC_LANE_SIZE);
	MoveLane((state->westbound == eGreen), VEHICLE_SYMBOL__WEST_BOUND, state->WestBound_Lane, CENTER_TRAFFIC_LANE_SIZE, state->WestBound_Queue, CENTER_TRAFFIC_LANE_SIZE);

	if ( state->center_eastbound == eGreen  &&  state->east_bound_counter > 0 ) {
		state->east_bound_counter--;
	}
	if ( state->westbound == eGreen  &&  state->west_bound_counter > 0 ) {
		state->west_bound_counter--;
	}
	if ( state->main_southbound == eGreen  &&  state->south_bound_counter > 0 ) {
		state->south_bound_counter--;
	}
	if ( state->main_northbound == eGreen  &&  state->north_bound_counter > 0 ) {
		state->north_bound_counter--;
	}
}

void LowTraffic_4WayStop(MachineState *state) {
	bool isMainStreet = ((state->stateStep % 2) == 0);
	if ( isMainStreet ) {
		printf("Main Street's red light turns on. Center Street's red light turns off.\n");
		state->westbound = state->center_eastbound = eOff;
		state->main_northbound = state->main_southbound = eRed;
	}

	if ( !isMainStreet ) {
		printf("Main Street's red light turns off. Center Street's red light turns on.\n");
		state->westbound = state->center_eastbound = eRed;
		state->main_northbound = state->main_southbound = eOff;
	}

	MoveTraffic(state);

	printf("%s", getTrafficSignalGraphic(state));

	if ( state->south_bound_counter > MAIN_STREET_MIN_QUEUE  ||  state->north_bound_counter > MAIN_STREET_MIN_QUEUE
			||  state->east_bound_counter > CENTER_STREET_MIN_QUEUE  ||  state->west_bound_counter > CENTER_STREET_MIN_QUEUE ) {
		state->intersectionState = e4WayStop_to_NormalOperation;
	}
}

void IncreasedTraffic_4WayStop_to_NormalOperations(MachineState *state) {
	if ( state->main_northbound == eGreen  &&  state->main_southbound == eGreen ) {
		printf("Main Street's red light turns off; green turns on. Center Street's red light turns on.\n");
		state->westbound = state->center_eastbound = eGreen;
		state->main_northbound = state->main_southbound = eRed;

	} else { // <-- !isMainStreet (or "isCenterStreet")
		printf("Main Street's red light turns on. Center Street's red light turns off; green turns on.\n");
		state->westbound = state->center_eastbound = eRed;
		state->main_northbound = state->main_southbound = eGreen;
	}

	state->greenCountdown = DEFAULT_GREEN_SECS;
	state->intersectionState = eNormalOperation;
	printf("%s", getTrafficSignalGraphic(state));
}

void NormalTraffic_GreenLightCountdown(MachineState *state) {
	state->greenCountdown--;

	printf("%s", getTrafficSignalGraphic(state));

//--- Transition to "red"
	if ( state->greenCountdown <= 0 ) {
			if ( state->main_northbound == eYellow  &&  state->main_southbound == eYellow ) {
				state->westbound = state->center_eastbound = eGreen;
				state->main_northbound = state->main_southbound = eRed;

			} else { // <-- Center Street
				state->westbound = state->center_eastbound = eRed;
				state->main_northbound = state->main_southbound = eGreen;
			}
			state->greenCountdown = DEFAULT_GREEN_SECS;

//--- Transition "yellow" (thru-moving traffic)
	} else if ( isBetween(0, state->greenCountdown, YELLOW_DURATION_DEFAULT) ) {
		if ( state->main_northbound != eRed  &&  state->main_southbound != eRed ) {

//			state->center_west = state->center_east = eRed; // <-- no need.
			state->main_northbound = state->main_southbound = eYellow;

		} else { // <-- Center Street
			state->westbound = state->center_eastbound = eYellow;
//			state->main_north = state->main_south = eRed; // <-- no need.
		}

	} else {//--- Operate under "green" state (thru-moving traffic)
		MoveTraffic(state);
	}
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
			state->south_bound_counter++;
			state->SouthBound_Queue[0] = VEHICLE_SYMBOL__SOUTH_BOUND;
		}
		if ( isBetween(0, random() % 100, 25) ) {
			state->north_bound_counter++;
			state->NorthBound_Queue[0] = VEHICLE_SYMBOL__NORTH_BOUND;
		}
		if ( isBetween(0, random() % 100, 40) ) {
			state->east_bound_counter++;
			state->EastBound_Queue[0] = VEHICLE_SYMBOL__EAST_BOUND;
		}
		if ( isBetween(0, random() % 100, 40) ) {
printf("!!! West bound!!!\n");
			state->west_bound_counter++;
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
//
//			} else if ( strcmp(*args, "--preferred=main") == 0 ) {
//				preferred = &centerStreet;
//
//			} else if ( strcmp(*args, "--preferred=center") == 0 ) {
//				preferred = &mainStreet;
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

//=================================================================================================================================
//--- UNIT TESTS ------------------------------------------------------------------------------------------------------------------
