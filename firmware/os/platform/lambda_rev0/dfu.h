#ifndef PLATFORM_LAMBDA_REV0_DFU_H_INC
#define PLATFORM_LAMBDA_REV0_DFU_H_INC
/*
	Device firmware update functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, January 2012.
*/

#include <string.h>

#include <cpu/cpu.h>
#include <include/error.h>
#include <include/types.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/memory/memorymap.h>
#include <kernel/util/kutil.h>


#define FLASH_OFFSET(x)		*((vu16 *) (ROM_START + (x << 1)))
#define FLASH_REG1			*((vu16 *) (ROM_START + (0x555 << 1)))
#define FLASH_REG2			*((vu16 *) (ROM_START + (0x2aa << 1)))


s32 dfu(ku16 *data, ku32 len);

#endif

