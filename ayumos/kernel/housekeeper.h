#ifndef KERNEL_HOUSEKEEPER_H_INC
#define KERNEL_HOUSEKEEPER_H_INC
/*
    Housekeeper process - runs on a timer interrupt, performs background tasks.

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


void housekeeper(void *arg);

#endif
