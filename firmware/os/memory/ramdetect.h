#ifndef MEMORY_RAMDETECT_H_INC
#define MEMORY_RAMDETECT_H_INC
/*
    RAM detection algorithm

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <include/defs.h>
#include <include/types.h>
#include <memory/memorymap.h>


u32 ram_detect(void);


#endif // OS_MEMORY_RAMDETECT_H
