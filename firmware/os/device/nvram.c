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
    u32 len = sizeof(nvram_bpb_t);

    nvram = dev_find("nvram0");
    if(nvram == NULL)
        return ENOSYS;

    /* Read the parameter block from battery-backed RAM */
    ret = nvram->read(nvram, 0, &len, pbpb);
    if(ret != SUCCESS)
        return ret;

    /*
        Verify checksum by computing the checksum of all fields in bbrpb with the exception of the
        final u16 field (which holds the stored checksum).
    */
    checksum = CHECKSUM16(pbpb, len - sizeof(u16));

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
    u32 len = sizeof(nvram_bpb_t);

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
    pbpb->checksum = CHECKSUM16(pbpb, len - sizeof(u16));

    return nvram->write(nvram, 0, &len, pbpb);
}
