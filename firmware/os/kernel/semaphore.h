#ifndef KERNEL_SEMAPHORE_H_INC
#define KERNEL_SEMAPHORE_H_INC
/*
    Kernel lock and semaphore functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <include/defs.h>
#include <include/types.h>


typedef u8 * sem_t;

s32 sem_init(sem_t *sem);
void sem_destroy(sem_t sem);
s32 sem_acquire(sem_t sem);
s32 sem_release(sem_t sem);

#endif
