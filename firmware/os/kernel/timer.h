#ifndef KERNEL_TIMER_H_INC
#define KERNEL_TIMER_H_INC
/*
    timer.h: periodic timer management functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>

/* Typedef defining a function handle timer ticks */
typedef void(*tick_handler_fn_t)();

/* Typedef defining a function to be called every tick by the default tick handler */
typedef void(*timer_callback_fn_t)(void *arg);


typedef struct timer_callback timer_callback_t;
struct timer_callback
{
    timer_callback_fn_t fn;
    void *arg;
    timer_callback_t *next;
};


s32 timer_init();
void timer_tick();
u32 timer_get_ticks();
s32 timer_add_callback(timer_callback_fn_t fn, void *arg);
s32 timer_remove_callback(timer_callback_fn_t fn);

#endif
