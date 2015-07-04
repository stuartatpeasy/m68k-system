#ifndef _BLOCK_HARNESS_H_
#define _BLOCK_HARNESS_H_

#define BLOCK_SIZE		(512)

#include "include/types.h"

u32 block_init(const char * const filename);
u32 block_shutdown();
u32 block_read(ku32 block_num, void* const buf);
u32 block_read_multi(u32 first_block_num, u32 num_blocks, void* buf);
u32 block_write(ku32 block_num, const void* const buf);
u32 block_write_multi(u32 first_block_num, u32 num_blocks, const void* buf);


#endif

