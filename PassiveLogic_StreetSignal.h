/*
 * PassiveLogic_StreetSignal.h
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */

#ifndef PASSIVELOGIC_STREETSIGNAL_H_
#define PASSIVELOGIC_STREETSIGNAL_H_

#define nullptr NULL
typedef enum { false, true } bool;

#define _RED_       "\x1B[31m"
#define _YELLOW_    "\x1B[93m"
#define _GREEN_     "\x1B[32m"
#define _UNDERLINE_ "\x1B[4m"
#define _RESET_     "\x1B[0m"

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

void parse_args(const char *args[], MachineState *state);
bool isBetween(int low, int test, int high);
bool str_startsWith(const char *str, const char *sub);
const char *getSemaphoreGraphic(MachineState *state);

#endif /* PASSIVELOGIC_STREETSIGNAL_H_ */
