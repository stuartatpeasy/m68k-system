#ifndef MONITOR_READLINE_H_INC
#define MONITOR_READLINE_H_INC
/*
	Implementation of the readline() function: reads a line from the serial port

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "device/duart.h"
#include "include/types.h"


void readline(char *buffer, ku32 buf_len, ku32 echo);

#endif

