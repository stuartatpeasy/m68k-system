#ifndef KERNEL_ERROR_H_INC
#define KERNEL_ERROR_H_INC
/*
    Kernel error-reporting functions

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


void kernel_warning(const char * const msg);
void kernel_fatal(const char * const msg) __attribute__((noreturn));

#endif
