/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "fat.h"
#include "fs/vfs.h"

vfs_driver_t g_fat_ops =
{
    .name = "fat",
    .init = fat_init,
    .mount = fat_mount,
    .umount = fat_umount,
    .fsnode_get = fat_fsnode_get,
    .fsnode_put = fat_fsnode_put,
    .locate = fat_locate,
    .stat = fat_stat
};


s32 fat_init()
{
    /* Nothing to do here */
    return SUCCESS;
}


s32 fat_mount(vfs_t *vfs)
{
    struct fat_bpb_block bpb;
    s32 ret;

    /*
        validate "superblock"
        allocate useful data structure in vfs->data
        store stuff in useful data structure
        maybe cache # of first data block, etc?
    */

    /* Read first sector */
    ret = device_read(vfs->dev, 0, 1, &bpb);
    if(ret != SUCCESS)
        return ret;

    { /* HACK - swap bytes in the sector we just read */
        s32 i;
        u16 *p = (u16 *) &bpb;
        for(i = sizeof(bpb) >> 1; i--; ++p)
            *p = ((*p >> 8) | (*p << 8));
    }

    dump_hex(&bpb, 1, 0, 512);

    /* Validate jump entry */
    if((bpb.jmp[0] != 0xeb) || (bpb.jmp[2] != 0x90))
    {
        printf("%s: bad FAT superblock: incorrect jump bytes: expected 0xeb 0xxx 0x90, "
               "read 0x%02x 0xxx 0x%02x\n", vfs->dev->name, bpb.jmp[0], bpb.jmp[2]);
        return 1;   /* FIXME - return code */
    }

    /* Validate partition signature */
    if(LE2N16(bpb.partition_signature) != 0xaa55)
    {
        printf("%s: bad FAT superblock: incorrect partition signature: expected 0xaa55, "
               "read 0x%04x\n", vfs->dev->name, LE2N16(bpb.partition_signature));
        return 1;   /* FIXME - return code */
    }

    return 0;
}


s32 fat_umount(vfs_t *vfs)
{

    return 0;
}


s32 fat_fsnode_get(vfs_t *vfs, u32 node, fsnode_t * const fsn)
{

    return 0;
}


s32 fat_fsnode_put(vfs_t *vfs, u32 node, const fsnode_t * const fsn)
{

    return 0;
}


u32 fat_locate(vfs_t *vfs, u32 node, const char * const path)
{
    /*

    */

    return 0;
}


s32 fat_stat(vfs_t *vfs, fs_stat_t *st)
{

    return 0;
}
