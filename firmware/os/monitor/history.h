#ifndef __MONITOR_ENV_H__
#define __MONITOR_ENV_H__
/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/


#include "include/defs.h"
#include "memory/kmalloc.h"

#define MONITOR_HISTORY_LEN     (10)

const char **g_history;

void history_init(void);
void history_clear(void);
void history_add(const char *cmd);
inline s32 history_get_len(void);
const char *history_get_at(const int where);
void history_remove(const int where);


#endif
