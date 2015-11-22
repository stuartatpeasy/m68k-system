#ifndef KERNEL_DEVICE_BLOCK_H_INC
#define KERNEL_DEVICE_BLOCK_H_INC
/*
	Block device generic functions and declarations

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.
*/


#include <kernel/include/defs.h>
#include <kernel/include/types.h>


typedef struct blockdev_stats
{
    u32 blocks_read;
    u32 blocks_written;
} blockdev_stats_t;

#endif
