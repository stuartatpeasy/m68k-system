#ifndef KERNEL_DEVICE_BLOCK_H_INC
#define KERNEL_DEVICE_BLOCK_H_INC
/*
	Block device generic functions and declarations

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.
*/


#include "include/defs.h"
#include "include/types.h"

struct blockdev_stats
{
    u32 blocks_read;
    u32 blocks_written;
};

typedef struct blockdev_stats blockdev_stats_t;

#endif
