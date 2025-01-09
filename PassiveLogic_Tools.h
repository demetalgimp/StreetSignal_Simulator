//tabstop=4
/*
 * PassiveLogic_Tools.h
 *
 *  Created on: Jan 8, 2025
 *      Author: swalton
 */

#ifndef PASSIVELOGIC_TOOLS_H_
#define PASSIVELOGIC_TOOLS_H_

#include <stdio.h>

#define nullptr NULL
typedef enum { false, true } bool;

bool str_startsWith(const char *str, const char *sub);
bool isBetween(int low, int test, int high);
int strcnt(const char *str, char c);

#endif /* PASSIVELOGIC_TOOLS_H_ */
