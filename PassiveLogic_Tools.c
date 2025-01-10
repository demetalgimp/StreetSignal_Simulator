//tabstop=4
/*
 * PassiveLogic_Tools.c
 *
 *  Created on: Jan 8, 2025
 *      Author: swalton
 */

#include "PassiveLogic_Tools.h"

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
	}
	return false;
}

bool isBetween(int low, int test, int high) {
	return ( low < test  &&  test < high );
}

int str_countChars(const char *str, char c) {
	int cnt = 0;
	if ( str != nullptr ) {
		while ( *str != 0 ) {
			if ( *str++ == c ) {
				cnt++;
			}
		}
	}
	return cnt;
}
