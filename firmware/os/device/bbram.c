/*
	Functions for managing battery-backed RAM

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, August 2015.
*/

#include "device/bbram.h"


s32 bbram_param_block_read(bbram_param_block_t *pparam_block)
{
    u16 checksum;

    /* Read the parameter block from battery-backed RAM */
    ds17485_user_ram_read(0, sizeof(bbram_param_block_t), pparam_block);

    /*
        Verify checksum by computing the checksum of all fields in bbrpb with the exception of the
        final u16 field (which holds the stored checksum).
    */

    checksum = CHECKSUM16(pparam_block, sizeof(bbram_param_block_t) - sizeof(u16));

    if((pparam_block->checksum != checksum) || (pparam_block->magic != BBRAM_PARAM_BLOCK_MAGIC))
        return ECKSUM;

    return SUCCESS;
}


s32 bbram_param_block_write(bbram_param_block_t *pparam_block)
{
    struct rtc_time tm;

    ds17485_get_time(&tm);
    rtc_time_to_timestamp(&tm, &pparam_block->mdate);

    pparam_block->magic = BBRAM_PARAM_BLOCK_MAGIC;
    pparam_block->checksum = CHECKSUM16(pparam_block, sizeof(bbram_param_block_t) - sizeof(u16));

    ds17485_user_ram_write(0, sizeof(bbram_param_block_t), pparam_block);

    return SUCCESS;
}
