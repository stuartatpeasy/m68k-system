#ifndef KLIBC_ERRNO_H_INC
#define KLIBC_ERRNO_H_INC
/*
    errno.h - declarations relating to the errno global; also #includes error definitions.

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, 2011-07-01.
*/

#include <kernel/include/types.h>
#include <klibc/include/errors.h>


extern s32 errno;

typedef s32 error_t;

#endif

