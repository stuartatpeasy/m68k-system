/*
    Kernel lock and semaphore functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/semaphore.h>
#include <cpu/cpu.h>


/*
    sem_init() - initialise a semaphore object.
*/
s32 sem_init(sem_t *sem)
{
    *sem = kcalloc(1, 1);
    return SUCCESS;
}


/*
    sem_destroy() - destroy a semaphore object.
*/
void sem_destroy(sem_t sem)
{
    kfree(sem);
}


/*
    sem_acquire() - attempt to acquire a semaphore.  SUCCESS = semaphore acquired; EAGAIN =
    semaphore already locked.
*/
s32 sem_acquire(sem_t sem)
{
    return cpu_tas(sem) ? EAGAIN : SUCCESS;
}


/*
    sem_release() - release a previously-init'ed semaphore.
*/
s32 sem_release(sem_t sem)
{
    *sem = 0;
    return SUCCESS;
}
