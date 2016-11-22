/*
    Kernel lock and semaphore functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/cpu.h>
#include <kernel/semaphore.h>


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
    sem_acquire_busy() - busy-loop until the specified semaphore can be acquired.
*/
s32 sem_acquire_busy(sem_t sem)
{
    s32 ret;

    do
    {
        ret = sem_try_acquire(sem);
    } while(ret == EAGAIN);

    return ret;
}


/*
    sem_acquire() - attempt to acquire the specified semaphore, yielding the current task's quantum
    repeatedly until the semaphore can be acquired.
*/
s32 sem_acquire(sem_t sem)
{
    s32 ret;

    while(1)
    {
        ret = sem_try_acquire(sem);
        if(ret == EAGAIN)
            cpu_switch_process();
        else
            return ret;
    }
}


/*
    sem_release() - release a previously-init'ed semaphore.
*/
s32 sem_release(sem_t sem)
{
    *sem = 0;
    return SUCCESS;
}
