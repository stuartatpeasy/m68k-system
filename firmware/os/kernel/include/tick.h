#ifndef KERNEL_INCLUDE_TICK_H_INC
#define KERNEL_INCLUDE_TICK_H_INC
/*
    tick.h: periodic timer management functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/* Function to handle timer ticks - the hardware timer causes this fn to be called once per tick */
typedef void(*tick_handler_fn_t)();

/* Typedef defining a function to be called by the default tick handler function */
typedef void(*tick_callback_fn_t)(void *arg);

/* Handle to a tick callback function */
typedef u32 tick_fn_t;


typedef struct tick_callback tick_callback_t;
struct tick_callback
{
    tick_fn_t           id;
    tick_callback_fn_t  fn;
    void                *arg;       /* Arg to be passed when fn is called               */
    u32                 interval;   /* Number of ticks between each invocation of fn    */
    u32                 counter;    /* Number of ticks until next invocation of fn      */
    tick_callback_t     *next;
};


s32 tick_init();
void tick();
u32 get_ticks();
s32 tick_add_callback(tick_callback_fn_t fn, void *arg, ku32 interval, tick_fn_t *id);
s32 tick_remove_callback(const tick_fn_t id);

#endif
