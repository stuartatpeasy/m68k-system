#ifndef DEVICE_BBRAM_H_INC
#define DEVICE_BBRAM_H_INC
/*
	Functions for managing battery-backed RAM

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, August 2015.
*/

#include "device/ds17485.h"
#include "include/defs.h"
#include "include/error.h"
#include "include/types.h"
#include "kutil/kutil.h"


#define BBRAM_PARAM_BLOCK_MAGIC     (0xfea51b1e)

struct bbram_param_block
{
    u32 magic;
    s8 rootfs[8];
    s8 fstype[8];
    s32 mdate;
    u16 checksum;
} __attribute__((packed));

typedef struct bbram_param_block bbram_param_block_t;

s32 bbram_param_block_read(bbram_param_block_t *pparam_block);
s32 bbram_param_block_write(bbram_param_block_t *pparam_block);

#endif
