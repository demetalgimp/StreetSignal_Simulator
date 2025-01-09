//tabstop=4
/*
 * PassiveLogic_StreetSignal.h
 *
 *  Created on: Jan 7, 2025
 *      Author: swalton
 */

#ifndef PASSIVELOGIC_STREETSIGNAL_H_
#define PASSIVELOGIC_STREETSIGNAL_H_

#include "PassiveLogic_Tools.h"

#define _RED_       "\x1B[31m"
#define _YELLOW_    "\x1B[93m"
#define _GREEN_     "\x1B[32m"
#define _UNDERLINE_ "\x1B[4m"
#define _RESET_     "\x1B[0m"

#define VEHICLE_SYMBOL__EAST_BOUND '>'
#define VEHICLE_SYMBOL__WEST_BOUND '<'
#define VEHICLE_SYMBOL__NORTH_BOUND '^'
#define VEHICLE_SYMBOL__SOUTH_BOUND 'v'

//=== STATES ======================================================================================================================
typedef enum { eOff = 0, eGreen, eYellow, eRed } SemaphoreColor;
typedef enum {
	eUnknown = 0,
	e4WayStop,
	e4WayStop_to_NormalOperation,
	eNormalOperation,
	eNormalOperation_to_4WayStop, // TODO
} LightState;

#define CENTER_TRAFFIC_LANE_SIZE	10
#define STATE_TRAFFIC_LANE_SIZE		7

#define QUEUE_SIZE	10
#define LANE_SIZE	7

typedef struct {
	char Symbol;
	SemaphoreColor color;
	enum { eApproachingIntersection, eLeavingIntersection } Direction;
	char Queue[QUEUE_SIZE];
	char Lane[LANE_SIZE];
} TrafficLane;

typedef struct {
//	bool isMainStreet;
	LightState intersectionState;
	int stateStep;

	int greenCountdown;

	TrafficLane west_bound;
	TrafficLane east_bound;
	TrafficLane north_bound;
	TrafficLane south_bound;

	SemaphoreColor westbound;
	int east_bound_counter;
	char WestBound_Queue[CENTER_TRAFFIC_LANE_SIZE + 1];
	char WestBound_Lane[CENTER_TRAFFIC_LANE_SIZE + 1];

	SemaphoreColor center_eastbound;
	int west_bound_counter;
	char EastBound_Queue[CENTER_TRAFFIC_LANE_SIZE + 1];
	char EastBound_Lane[CENTER_TRAFFIC_LANE_SIZE + 1];

	SemaphoreColor main_northbound;
	int south_bound_counter;
	char NorthBound_Queue[STATE_TRAFFIC_LANE_SIZE + 1];
	char NorthBound_Lane[STATE_TRAFFIC_LANE_SIZE + 1];

	SemaphoreColor main_southbound;
	int north_bound_counter;
	char SouthBound_Queue[STATE_TRAFFIC_LANE_SIZE + 1];
	char SouthBound_Lane[STATE_TRAFFIC_LANE_SIZE + 1];

//	int green_duration;
//	int yellow_duration;
} MachineState;

//--- For unit testing
void MachineState_init(MachineState *state);
void parse_args(const char *args[], MachineState *state);
const char *getTrafficSignalGraphic(MachineState *state);
void MoveLane(bool hasGreenLight, char vehicle_symbol, char *lane, int lane_size, char *queue, int queue_size);

#endif /* PASSIVELOGIC_STREETSIGNAL_H_ */
