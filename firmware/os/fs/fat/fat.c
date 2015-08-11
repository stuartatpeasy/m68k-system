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
    struct fat_fs *fs;
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

    /* Validate jump entry */
    if((bpb.jmp[0] != 0xeb) || (bpb.jmp[2] != 0x90))
    {
        printf("%s: bad FAT superblock: incorrect jump bytes: expected 0xeb 0xxx 0x90, "
               "read 0x%02x 0xxx 0x%02x\n", vfs->dev->name, bpb.jmp[0], bpb.jmp[2]);
        return EBADSBLK;
    }

    /* Validate partition signature */
    if(LE2N16(bpb.partition_signature) != FAT_PARTITION_SIG)
    {
        printf("%s: bad FAT superblock: incorrect partition signature: expected 0x%04x, "
               "read 0x%04x\n", vfs->dev->name, FAT_PARTITION_SIG, LE2N16(bpb.partition_signature));
        return EBADSBLK;
    }

    fs = kmalloc(sizeof(struct fat_fs));
    if(!fs)
        return ENOMEM;

    /* Precalculate some useful figures.  TODO: maybe some validation on these numbers? */

    /* FIXME - use BLOCK_SIZE everywhere?  e.g. retire ATA_SECTOR_SIZE, etc. */
    if(LE2N16(bpb.bytes_per_sector) != BLOCK_SIZE)
    {
        printf("%s: bad FAT sector size %d; can only handle %d-byte sectors\n",
               vfs->dev->name, LE2N16(bpb.bytes_per_sector), BLOCK_SIZE);
        return EBADSBLK;
    }

    fs->sectors_per_cluster = bpb.sectors_per_cluster;
    fs->bytes_per_cluster = bpb.sectors_per_cluster * LE2N16(bpb.bytes_per_sector);
    fs->nsectors = LE2N16(bpb.sectors_per_fat);
    fs->root_dir_sectors = ((LE2N16(bpb.root_entry_count) * 32)
                            + (LE2N16(bpb.bytes_per_sector) - 1)) / LE2N16(bpb.bytes_per_sector);
    fs->first_data_sector = LE2N16(bpb.reserved_sectors) + (bpb.fats * fs->nsectors)
                            + fs->root_dir_sectors;

    printf("FAT superblock OK\n"
           "- sectors_per_cluster = %d\n"
           "- bytes per cluster   = %d\n"
           "- nsectors            = %d\n"
           "- root_dir_sectors    = %d\n"
           "- first_data_sector   = %d\n",
           fs->sectors_per_cluster, fs->bytes_per_cluster, fs->nsectors, fs->root_dir_sectors,
           fs->first_data_sector);

    vfs->data = fs;

    return 0;
}


s32 fat_umount(vfs_t *vfs)
{
    /* TODO: (both of these tasks may not be this module's responsibility):
        - flush all dirty sectors
        - ensure no open file handles exist on this fs
    */
    kfree(vfs->data);

    return SUCCESS;
}


/*
    fat_read_node() - read cluster "node" into "buffer"
*/
s32 fat_read_node(vfs_t *vfs, u32 node, void *buffer)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;

    return device_read(vfs->dev, ((node - 2) * fs->sectors_per_cluster) + fs->first_data_sector,
                       fs->sectors_per_cluster, buffer);
}


/*
    fat_get_next_node() - given a node, find the next node in the chain
*/
s32 fat_get_next_node(vfs_t *vfs, u32 node, u32 *next_node)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    fat16_cluster_id sector[BLOCK_SIZE / sizeof(fat16_cluster_id)];

    /* Compute offset from start of FAT, in bytes, of the specified node */
    u32 fat_offset = node * sizeof(fat16_cluster_id);

    /* Determine which sector contains the bytes specified by fat_offset */
    u32 fat_sector = fs->first_fat_sector + (fat_offset >> LOG_BLOCK_SIZE);

    /* Calculate the offset into the sector */
    u32 ent_offset = (fat_offset & (BLOCK_SIZE - 1)) >> 1;

    /* Read sector */
    u32 ret = device_read(vfs->dev, fat_sector, 1, &sector);
    if(ret != SUCCESS)
        return ret;

    *next_node = sector[ent_offset];

    return SUCCESS;
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
    /* given a node and a path component, return the node containing that component */

    /*
        while(node != end_of_chain)
        {
            read(node)
            scan node for path
            found?
                return corresponding node
            not found?
                node = get_next_node_in_chain(node)
        }
        return not_found
    */
    s32 ret;
    void *buffer;

    /* Allocate a buffer to hold a cluster */
    buffer = kmalloc(((fat_fs_t *) vfs->data)->bytes_per_cluster;
    if(!buffer)
        return ENOMEM;

    /* Iterate over the clusters in the chain */
    while(!FAT_NO_MORE_CLUSTERS(node))
    {
        /* Read the node */
        ret = fat_read_node(vfs, node, buffer);
        if(ret != SUCCESS)
        {
            kfree(buffer);
            return ret;
        }

        /* Scan the node to see whether it contains "path" */
        /* TODO */

        /* Find the next node in the chain */
        ret = fat_get_next_node(vfs, node, &node);
        if(ret != SUCCESS)
        {
            kfree(buffer);
            return ret;
        }
    }

    kfree(buffer);

    return ENOENT;
}


s32 fat_stat(vfs_t *vfs, fs_stat_t *st)
{

    return 0;
}
