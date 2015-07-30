#ifndef __CPU_UTILITIES_H__
#define __CPU_UTILITIES_H__
/*
	MC68010 CPU utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "stdio.h"
#include "include/types.h"
#include "cpu/exceptionstackframes.h"

#include <stdio.h>

#define cpu_nop()		do { asm volatile("nop" : ); } while(0)

/* Disable interrupts by setting the IRQ mask to 7 in the SR. */
#define cpu_disable_interrupts()    \
    asm volatile                    \
    (                               \
        "oriw #0x0700, %%sr\n"      \
        :                           \
        :                           \
    )                               \

/* Enable interrupts by setting the IRQ mask to 0 in the SR. */
#define cpu_enable_interrupts()     \
    asm volatile                    \
    (                               \
        "andiw #0xf8ff, %%sr\n"     \
        :                           \
        :                           \
    )                               \

void cpu_halt(void) __attribute__ ((noreturn));

const char * const cpu_dump_status_register(ku16 sr);
void cpu_dump_exc_frame(const struct mc68010_exc_frame * const f);
void cpu_dump_address_exc_frame(const struct mc68010_address_exc_frame * const f);

#endif
