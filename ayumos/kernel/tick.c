/*
    tick.h: periodic timer management functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2016.
*/

#include <kernel/include/device/device.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/preempt.h>
#include <kernel/include/tick.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>


static tick_callback_t *callbacks = NULL;
static u32 tick_count = 0;
static tick_fn_t next_id = 0;   /* TODO: find a better way of generating IDs.  This may overflow */
static dev_t *timer = NULL;


/*
    tick_init() - find a suitable timer device; register timer_tick() as the tick handler.
    TODO: we assume that we'll use the first timer in the system (timer0).  This may not be ideal.
*/
s32 tick_init()
{
    ku32 requested_freq = TICK_RATE, enable = 1;
    u32 actual_freq = 0;
    const char * const dev_name = "timer0";
    s32 ret;

    /* Locate the timer device */
    timer = dev_find(dev_name);
    if(!timer)
    {
        puts("timer_init: no timer device found");
        return ENODEV;
    }

    /* Set timer frequency */
    ret = timer->control(timer, dc_timer_set_freq, &requested_freq, &actual_freq);
    if(ret == SUCCESS)
    {
        printf("timer_init: %s: tick rate %dHz requested; %dHz actual\n",
               dev_name, requested_freq, actual_freq);
    }
    else
    {
        printf("timer_init: %s: requested tick rate %dHz; error: %s\n",
               dev_name, requested_freq, kstrerror(ret));
        return ret;
    }

    /* Register tick handler function */
    ret = timer->control(timer, dc_timer_set_tick_fn, tick, NULL);
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
    tick() - called at every timer "tick".  Performs scheduled tasks.  Note: should be called
    outside of IRQ context.
*/
void tick()
{
    tick_callback_t *item;
    u32 enable = 0;
    s32 ret;

    /* Disable the timer */
    ret = timer->control(timer, dc_timer_set_enable, &enable, NULL);
    if(ret == SUCCESS)
    {
        ++tick_count;

        preempt_disable();

        /* Handle per-tick functions */
        for(item = callbacks; item; item = item->next)
        {
            if(!--item->counter)
            {
                item->counter = item->interval;
                item->fn(item->arg);
            }
        }

        preempt_enable();

        /* Re-enable the timer */
        enable = 1;
        ret = timer->control(timer, dc_timer_set_enable, &enable, NULL);
        if(ret != SUCCESS)
            printf("timer_tick: failed to re-enable timer: %s\n", kstrerror(ret));
    }
}


/*
    get_ticks() - get the current "tick count"
*/
u32 get_ticks()
{
    return tick_count;
}


/*
    tick_add_callback() - add a callback function to the list of functions called periodically.
    The callback function will be called every <interval> ticks.
*/
s32 tick_add_callback(tick_callback_fn_t fn, void *arg, ku32 interval, tick_fn_t *id)
{
    tick_callback_t *cbnew;

    if(!interval)
        return EINVAL;

    cbnew = (tick_callback_t *) slab_alloc(sizeof(tick_callback_t));
    if(!cbnew)
        return ENOMEM;

    cbnew->id       = ++next_id;
    cbnew->fn       = fn;
    cbnew->arg      = arg;
    cbnew->next     = NULL;
    cbnew->interval = interval;
    cbnew->counter  = interval;

    preempt_disable();

    if(!callbacks)
        callbacks = cbnew;
    else
    {
        tick_callback_t *p;
        for(p = callbacks; p->next; p = p->next)
            ;

        p->next = cbnew;
    }

    preempt_enable();

    if(*id)
        *id = cbnew->id;

    return SUCCESS;
}


/*
    tick_remove_callback() - remove the specified callback function from the list of functions
    called every tick.
*/
s32 tick_remove_callback(const tick_fn_t id)
{
    tick_callback_t *p, *prev;

    preempt_disable();

    if(callbacks)
    {
        for(prev = NULL, p = callbacks; p; prev = p, p = p->next)
        {
            if(p->id == id)
            {
                if(!prev)
                    callbacks = p->next;    /* Deleting the first item in the list */
                else
                    prev->next = p->next;

                preempt_enable();
                slab_free(p);

                return SUCCESS;
            }
        }
    }

    preempt_enable();
    return ENOENT;
}
