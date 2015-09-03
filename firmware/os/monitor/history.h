#ifndef MONITOR_HISTORY_H_INC
#define MONITOR_HISTORY_H_INC
/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/


#include "include/defs.h"
#include "include/types.h"
#include "memory/kmalloc.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>


struct command_history
{
    char **item;
    s32 next;
    s32 len;
};

typedef struct command_history command_history_t;


s32 history_init(command_history_t **h, s32 len);
s32 history_destroy(command_history_t *h);
void history_clear(command_history_t *h);
void history_add(command_history_t *h, const char *cmd);
const char *history_get_at(command_history_t *h, s32 where);
u32 history_get_len(command_history_t *h);

#endif
