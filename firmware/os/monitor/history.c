/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/

#include "monitor/history.h"

const char **g_history;
int g_history_start;


void history_init(void)
{
    g_history = kmalloc(MONITOR_HISTORY_LEN * sizeof(char *));
    g_history_start = 0;
}


void history_clear(void)
{

}


void history_add(const char *cmd)
{

}


inline s32 history_get_len(void)
{
    return MONITOR_HISTORY_LEN;
}


const char *history_get_at(const int where)
{
    if(where >= MONITOR_HISTORY_LEN)
        return NULL;

    return g_history[where - g_history_start];
}


void history_remove(const int where)
{

}
