/*
    Kernel lock and semaphore functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/cpu.h>
#include <kernel/include/semaphore.h>


/*
    sem_init() - initialise a semaphore object.
*/
s32 sem_init(sem_t *sem)
{
    *sem = 0;
    return SUCCESS;
}


/*
    sem_destroy() - destroy a semaphore object.
*/
void sem_destroy(sem_t *sem)
{
    /* Nothing to do here */
    UNUSED(sem);
}


/*
    sem_acquire_busy() - busy-loop until the specified semaphore can be acquired.
    This is effectively a spin-lock.
*/
void sem_acquire_busy(sem_t *sem)
{
    while(sem_try_acquire(sem) == EAGAIN)
        ;
}


/*
    sem_acquire() - attempt to acquire the specified semaphore, yielding the current task's quantum
    repeatedly until the semaphore can be acquired.
*/
void sem_acquire(sem_t *sem)
{
    while(sem_try_acquire(sem) == EAGAIN)
        cpu_switch_process();
}


/*
    sem_release() - release a previously-init'ed semaphore.
*/
void sem_release(sem_t *sem)
{
    *sem = 0;
}
