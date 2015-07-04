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

#define __cpu_nop()		do { asm volatile("nop" : ); } while(0)

inline void __cpu_set_irq_mask(u32 u);
inline void __cpu_halt(void) __attribute__ ((noreturn));

const char * const __cpu_dump_status_register(ku16 sr);
void __cpu_dump_exc_frame(const struct __mc68010_exc_frame * const f);
void __cpu_dump_address_exc_frame(const struct __mc68010_address_exc_frame * const f);

#endif
