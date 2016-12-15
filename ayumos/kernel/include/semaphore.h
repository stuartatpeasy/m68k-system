#ifndef KERNEL_INCLUDE_SEMAPHORE_H_INC
#define KERNEL_INCLUDE_SEMAPHORE_H_INC
/*
    Kernel lock and semaphore functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/cpu.h>
#include <klibc/include/errors.h>


typedef u8 sem_t;

s32 sem_init(sem_t *sem);
void sem_destroy(sem_t *sem);
void sem_acquire(sem_t *sem);
void sem_acquire_busy(sem_t *sem);
void sem_release(sem_t *sem);


/*
    sem_acquire() - attempt to acquire a semaphore.  SUCCESS = semaphore acquired; EAGAIN =
    semaphore already locked.
*/
inline s32 sem_try_acquire(sem_t *sem)
{
    return cpu_tas(sem) ? EAGAIN : SUCCESS;
};

#endif
