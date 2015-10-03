#ifndef CPU_UTILITIES_H_INC
#define CPU_UTILITIES_H_INC
/*
	MC68010 CPU utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include <stdio.h>
#include "include/types.h"
#include "cpu/exceptionstackframes.h"


const char * const cpu_dump_status_register(ku16 sr);
void cpu_dump_exc_frame(const struct mc68010_exc_frame * const f);
void cpu_dump_address_exc_frame(const struct mc68010_address_exc_frame * const f);

#endif
