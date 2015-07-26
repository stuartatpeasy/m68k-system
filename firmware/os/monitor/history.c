/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/

#include "monitor/history.h"

char **g_history = NULL;
int g_history_pos = 0;


void history_init(void)
{
    g_history = malloc(MONITOR_HISTORY_LEN * sizeof(char *));
    bzero(g_history, sizeof(char *) * MONITOR_HISTORY_LEN);
}


void history_clear(void)
{
    int i;
    for(i = 0; i < MONITOR_HISTORY_LEN; i++)
    {
        if(g_history[i])
        {
            free(g_history[i]);
            g_history[i] = NULL;
        }
    }

    g_history_pos = 0;
}


void history_add(const char *cmd)
{
    if(g_history[g_history_pos])
        free(g_history[g_history_pos]);

    g_history[g_history_pos] = malloc(strlen(cmd) + 1);
    if(g_history[g_history_pos])
    {
        strcpy(g_history[g_history_pos], cmd);

        if(++g_history_pos == MONITOR_HISTORY_LEN)
        {
            g_history_pos = 0;
        }
    }
}


const char *history_get_at(int where)
{
    if(where >= MONITOR_HISTORY_LEN)
        return NULL;
/*
    where += g_history_pos + 1;
    if(where >= MONITOR_HISTORY_LEN)
        where -= MONITOR_HISTORY_LEN;
*/
    return g_history[where];
}
