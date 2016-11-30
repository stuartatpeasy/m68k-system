#ifndef KERNEL_INCLUDE_LOCK_H_INC
#define KERNEL_INCLUDE_LOCK_H_INC
/*
    Locking

    Part of ayumos


    (c) Stuart Wallace, November 2016.
*/

#include <kernel/cpu.h>


typedef vu32 lock_t;


/*
    Currently ayumos does not support multiple processors.  Locking is therefore quite
    straightforward; it boils down to disabling and re-enabling pre-emption.  Note that locking
    doesn't prevent NMIs.
*/


/* LOCK_INIT() macro enables lock objects to be initialised outside of function context */
#define LOCK_INIT()     (0)


/*
    lock_init() - initialise a lock object
*/
inline void lock_init(lock_t *lock)
{
    *lock = 0;
}


/*
    lock_enter() - enter a critical section by disabling interrupts and incrementing the lock count.
*/
inline void lock_enter(lock_t *lock)
{
    cpu_disable_interrupts();
    ++*lock;
}


/*
    lock_leave() - leave a critical section by decrementing the lock count and enabling interrupts
    if the lock count has reached zero.
*/
inline void lock_leave(lock_t *lock)
{
    if(!--*lock)
        cpu_enable_interrupts();
}

#endif
