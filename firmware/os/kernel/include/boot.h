#ifndef KERNEL_INCLUDE_BOOT_H_INC
#define KERNEL_INCLUDE_BOOT_H_INC
/*
	boot.h: declarations of functions useful during boot

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 17th October 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


void boot_early_fail(ku32 code);
void boot_list_mass_storage();
void boot_list_partitions();

#endif
