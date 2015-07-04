#ifndef __MONITOR_READLINE_H__
#define __MONITOR_READLINE_H__
/*
	Implementation of the readline() function: reads a line from the serial port

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "include/types.h"

#include "duart.h"


void readline(char *buffer, ku32 buf_len, ku32 echo);

#endif

