/*
	Command-history support for monitor system

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2015.
*/

#include "monitor/history.h"


/*
    history_init() - allocate a new command_history_t object and initialise it in readiness to hold
    len entries.
*/
s32 history_init(command_history_t **h, unsigned int len)
{
    *h = CHECKED_KMALLOC(sizeof(command_history_t));

    (*h)->item = kcalloc(len, sizeof(char *));
    if(!(*h)->item)
    {
        kfree(*h);
        return ENOMEM;
    }

    (*h)->start = 0;
    (*h)->next = 0;
    (*h)->len = len;

    return SUCCESS;
}


/*
    history_destroy() - destructor for a command_history_t object
*/
s32 history_destroy(command_history_t *h)
{
    history_clear(h);
    kfree(h);

    return SUCCESS;
}


/*
    history_clear() - free all items held in a command_history_t object
*/
void history_clear(command_history_t *h)
{
    u32 u;
    for(u = 0; u < h->len; u++)
        if(h->item[u])
            kfree(h->item[u]);

    bzero(h->item, sizeof(char *) * h->len);

    h->start = 0;
    h->next = 0;
}


/*
    history_add() - add an item to a command_history_t object
*/
void history_add(command_history_t *h, const char *cmd)
{
    if(h->item[h->next])
        free(h->item[h->next]);

    h->item[h->next] = kmalloc(strlen(cmd) + 1);
    if(h->item[h->next])
    {
        strcpy(h->item[h->next], cmd);

        if(++h->next == h->len)
            h->next = 0;

        if(h->next == h->start)
            if(++h->start == h->len)
                h->len = 0;
    }
}


/*
    history_get_at() - get a ptr to a particular item
*/
const char *history_get_at(command_history_t *h, u32 where)
{
    if(where >= h->len)
        return NULL;

    where += h->start;
    if(where > h->len)
        where -= h->len;

    return h->item[where];
}


/*
    history_get_len() - return the maximum number of items a history obj can hold
*/
u32 history_get_len(command_history_t *h)
{
    return h->len;
}
