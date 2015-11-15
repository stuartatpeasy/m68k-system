#ifndef DEVICE_NVRAM_H_INC
#define DEVICE_NVRAM_H_INC
/*
	Functions for managing a boot parameter block (BPB), stored in a non-volatile RAM block

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, August 2015.
*/

#include <device/device.h>
#include <include/defs.h>
#include <include/error.h>
#include <include/types.h>
#include <kernel/util/kutil.h>


#define NVRAM_BPB_MAGIC     (0xfea51b1e)

struct nvram_bpb
{
    u32 magic;
    s8 rootfs[8];
    s8 fstype[8];
    s32 mdate;
    u16 checksum;
} __attribute__((packed));

typedef struct nvram_bpb nvram_bpb_t;

s32 nvram_bpb_read(nvram_bpb_t *pbpb);
s32 nvram_bpb_write(nvram_bpb_t *pbpb);

#endif
