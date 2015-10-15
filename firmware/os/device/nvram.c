/*
	Functions for managing a boot parameter block (BPB), stored in a non-volatile RAM block

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, August 2015.
*/

#include "device/nvram.h"


/*
    nvram_bpb_read() - read a boot parameter block object from NVRAM
*/
s32 nvram_bpb_read(nvram_bpb_t *pbpb)
{
    s32 ret;
    u16 checksum;
    dev_t *nvram;

    nvram = dev_find("nvram0");
    if(nvram == NULL)
        return ENOSYS;

    /* Read the parameter block from battery-backed RAM */
    ret = ((nvram_ops_t *) nvram->driver)->read(nvram, 0, sizeof(nvram_bpb_t), pbpb);
    if(ret != SUCCESS)
        return ret;

    /*
        Verify checksum by computing the checksum of all fields in bbrpb with the exception of the
        final u16 field (which holds the stored checksum).
    */
    checksum = CHECKSUM16(pbpb, sizeof(nvram_bpb_t) - sizeof(u16));

    if((pbpb->checksum != checksum) || (pbpb->magic != NVRAM_BPB_MAGIC))
        return ECKSUM;

    return SUCCESS;
}


/*
    nvram_bpb_write() - write a boot parameter block object to NVRAM
*/
s32 nvram_bpb_write(nvram_bpb_t *pbpb)
{
    s32 ret;
    rtc_time_t tm;
    dev_t *nvram;

    nvram = dev_find("nvram0");
    if(nvram == NULL)
        return ENOSYS;

    ret = get_time(&tm);
    if(ret != SUCCESS)
        return ret;

    ret = rtc_time_to_timestamp(&tm, &pbpb->mdate);
    if(ret != SUCCESS)
        return ret;

    pbpb->magic = NVRAM_BPB_MAGIC;
    pbpb->checksum = CHECKSUM16(pbpb, sizeof(nvram_bpb_t) - sizeof(u16));

    return ((nvram_ops_t *) nvram->driver)->write(nvram, 0, sizeof(nvram_bpb_t), pbpb);
}