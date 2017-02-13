/*
    Environment-variable support for monitor system

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2015.
*/

#include <monitor/include/env.h>



void env_init(void)
{

}


void env_set(const char *key, const char *val)
{
    UNUSED(key);
    UNUSED(val);

    /* TODO */
}


const char *env_get(const char *key)
{
    UNUSED(key);

    /* TODO */

    return NULL;
}
