/*
    timer.h: periodic timer management functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2016.

    FIXME: all sorts of locking problems here, I expect.  Protect g_timer_callbacks with spinlock?
*/

#include <kernel/device/device.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/timer.h>
#include <kernel/util/kutil.h>
#include <klibc/stdio.h>


timer_callback_t *g_timer_callbacks = NULL;
u32 g_tick_count = 0;


/*
    timer_init() - find a suitable timer device; register timer_tick() as the tick handler.
    TODO: we assume that we'll use the first timer in the system (timer0).  This may not be ideal.
*/
s32 timer_init()
{
    ku32 requested_freq = TICK_RATE, enable = 1;
    u32 actual_freq = 0;
    const char * const dev_name = "timer0";
    s32 ret;

    /* Locate the timer device */
    dev_t *timer = dev_find(dev_name);
    if(!timer)
    {
        puts("timer_init: no timer device found");
        return ENODEV;
    }

    /* Set timer frequency */
    ret = timer->control(timer, dc_timer_set_freq, &requested_freq, &actual_freq);
    if(ret == SUCCESS)
    {
        printf("timer_init: %s: requested tick rate %dHz; actual tick rate %dHz\n",
               dev_name, requested_freq, actual_freq);
    }
    else
    {
        printf("timer_init: %s: requested tick rate %dHz; error: %s\n",
               dev_name, requested_freq, kstrerror(ret));
        return ret;
    }

    /* Register tick handler function */
    ret = timer->control(timer, dc_timer_set_tick_fn, timer_tick, NULL);
    if(ret != SUCCESS)
    {
        printf("timer_init: %s: failed to register tick function: %s\n",
               dev_name, kstrerror(ret));
        return ret;
    }

    /* Enable the timer */
    ret = timer->control(timer, dc_timer_set_enable, &enable, NULL);
    if(ret != SUCCESS)
    {
        printf("timer_init: %s: failed: %s\n", dev_name, kstrerror(ret));
        return ret;
    }

    return SUCCESS;
}


/*
    timer_tick() - called at every timer "tick".  Performs scheduled tasks.  Note: should be called
    outside of IRQ context.
*/
void timer_tick()
{
    timer_callback_t *item;

    ++g_tick_count;

    /* Handle per-tick functions */
    for(item = g_timer_callbacks; item; item = item->next)
        item->fn(item->arg);
}


/*
    timer_get_ticks() - get the current "tick count"
*/
u32 timer_get_ticks()
{
    return g_tick_count;
}


/*
    timer_add_callback() - add a callback function to the list of functions called every tick.
*/
s32 timer_add_callback(timer_callback_fn_t fn, void *arg)
{
    timer_callback_t *cbnew = (timer_callback_t *) kmalloc(sizeof(timer_callback_t));
    if(!cbnew)
        return ENOMEM;

    cbnew->fn = fn;
    cbnew->arg = arg;
    cbnew->next = NULL;

    if(!g_timer_callbacks)
        g_timer_callbacks = cbnew;
    else
    {
        timer_callback_t *p;
        for(p = g_timer_callbacks; p->next; p = p->next)
            ;

        p->next = cbnew;
    }

    return SUCCESS;
}


/*
    timer_remove_callback() - remove the specified callback function from the list of functions
    called every tick.
*/
s32 timer_remove_callback(timer_callback_fn_t fn)
{
    timer_callback_t *p, *prev;

    if(g_timer_callbacks)
    {
        for(prev = NULL, p = g_timer_callbacks; p; prev = p, p = p->next)
        {
            if(p->fn == fn)
            {
                if(!prev)
                    g_timer_callbacks = p->next;    /* Deleting the first item in the list */
                else
                    prev->next = p->next;

                kfree(p);
                return SUCCESS;
            }
        }
    }

    return ENOENT;
}
