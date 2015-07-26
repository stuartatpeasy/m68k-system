#ifndef OS_MONITOR_HISTORY_H
#define OS_MONITOR_HISTORY_H
/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/


#include "include/defs.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MONITOR_HISTORY_LEN     (10)

void history_init(void);
void history_clear(void);
void history_add(const char *cmd);
const char *history_get_at(int where);


#endif
