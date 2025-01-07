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
#define YELLOW_DURATION_DEFAULT 5

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

void init(void) {
}

//--- PROGRAM ENTRY ---------------------------------------------------------------------------------------------------------------
bool run = true;

#define CENTER_STREET_MIN_QUEUE 2
#define MAIN_STREET_MIN_QUEUE 2

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

void LowTraffic_4WayStop(MachineState *state) {
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

	if ( state->south_bound > MAIN_STREET_MIN_QUEUE  ||  state->north_bound > MAIN_STREET_MIN_QUEUE
			||  state->east_bound > CENTER_STREET_MIN_QUEUE  ||  state->west_bound > CENTER_STREET_MIN_QUEUE ) {
		state->intersectionState = e4Way_to_NormalOperation;
	}
}

void IncreasedTraffic_4WayStop_to_NormalOperations(MachineState *state) {
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
	state->intersectionState = eNormalOperation;
}

void NormalTraffic_GreenLightCountdown(MachineState *state) {
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

}

void state_loop(MachineState *state) {
	while ( run ) {
		switch ( state->intersectionState ) {
			case e4WayStop_noTraffic:
				LowTraffic_4WayStop(state);
				break;

			case e4Way_to_NormalOperation:
				IncreasedTraffic_4WayStop_to_NormalOperations(state);
				break;

			case eNormalOperation:
				NormalTraffic_GreenLightCountdown(state);
				break;

			default:
				fprintf(stderr, "ERROR! Cannot get here!\n");
				abort();
		}

		sleep(1);
		state->stateStep++;

		if ( isBetween(0, random() % 100, 40) ) {
			state->south_bound++;
		}
		if ( isBetween(0, random() % 100, 40) ) {
			state->north_bound++;
		}
		if ( isBetween(0, random() % 100, 60) ) {
			state->east_bound++;
		}
		if ( isBetween(0, random() % 100, 60) ) {
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
