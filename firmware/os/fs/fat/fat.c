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


/* HACK - in rev0 hardware, the ATA interface has its bytes the wrong way around.  This function
    swaps the endianness of each pair of bytes */
void *fat_hack_sector_swap_bytes(void *buffer, u32 num_sectors)
{
    s32 i;
    u16 *p = (u16 *) buffer;
    for(i = (num_sectors * BLOCK_SIZE) >> 1; i--; ++p)
        *p = ((*p >> 8) | (*p << 8));

    return buffer;
}


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

    /* Read first sector */
    ret = device_read(vfs->dev, 0, 1, &bpb);
    if(ret != SUCCESS)
        return ret;

    fat_hack_sector_swap_bytes(&bpb, 1);    /* HACK */

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

    /* FIXME - use BLOCK_SIZE everywhere?  e.g. retire ATA_SECTOR_SIZE, etc. */
    if(LE2N16(bpb.bytes_per_sector) != BLOCK_SIZE)
    {
        printf("%s: bad FAT sector size %d; can only handle %d-byte sectors\n",
               vfs->dev->name, LE2N16(bpb.bytes_per_sector), BLOCK_SIZE);
        return EBADSBLK;
    }

    /* Precalculate some useful figures.  TODO: maybe some validation on these numbers? */

    fs->sectors_per_cluster   = bpb.sectors_per_cluster;
    fs->bytes_per_cluster     = bpb.sectors_per_cluster * LE2N16(bpb.bytes_per_sector);
    fs->sectors_per_fat       = LE2N16(bpb.sectors_per_fat);

    fs->root_dir_sectors      = ((LE2N16(bpb.root_entry_count) * sizeof(struct fat_dirent))
                                    + (BLOCK_SIZE - 1)) >> LOG_BLOCK_SIZE;

    fs->first_fat_sector      = LE2N16(bpb.reserved_sectors);

    fs->first_data_sector     = LE2N16(bpb.reserved_sectors) + (bpb.fats * fs->sectors_per_fat)
                                    + fs->root_dir_sectors;

    fs->root_dir_first_sector = fs->first_data_sector - fs->root_dir_sectors;

    fs->total_sectors         = bpb.sectors_in_volume ? LE2N16(bpb.sectors_in_volume) :
                                    LE2N32(bpb.large_sectors);

    fs->total_data_sectors    = fs->total_sectors
                                    - (LE2N16(bpb.reserved_sectors)
                                       + (bpb.fats * fs->sectors_per_fat) + fs->root_dir_sectors);

    fs->total_clusters        = fs->total_data_sectors / fs->sectors_per_cluster;

    vfs->data = fs;

    fat_locate(vfs, 0, "blah");

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

    /* HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK */
    s32 ret = device_read(vfs->dev, ((node - 2) * fs->sectors_per_cluster) + fs->first_data_sector,
                          fs->sectors_per_cluster, buffer);

    fat_hack_sector_swap_bytes(buffer, fs->sectors_per_cluster);

    return ret;
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

    /* HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK */
    fat_hack_sector_swap_bytes(&sector, 1);

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

    s32 ret, dir_end, lfn_len;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    void *buffer;

    /* Allocate a buffer to hold a cluster */
    ku32 bytes_per_cluster = ((fat_fs_t *) vfs->data)->bytes_per_cluster;
    buffer = kmalloc(bytes_per_cluster);
    if(!buffer)
        return ENOMEM;

    /* Iterate over the clusters in the chain */
    dir_end = 0;
    lfn_len = 0;
    while(!FAT_NO_MORE_CLUSTERS(node))
    {
        fat_dirent_t *de;

        /* Read the node */
        ret = fat_read_node(vfs, node, buffer);
        if(ret != SUCCESS)
        {
            kfree(buffer);
            return ret;
        }

        /* Scan the node to see whether it contains "path" */
        for(de = (fat_dirent_t *) buffer; (void *) de < (buffer + bytes_per_cluster); ++de)
        {
            if(de->file_name[0] == 0x00)
            {
                dir_end = 1;    /* No more entries in this directory */
                break;
            }
            else if((u8) de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;
            else if(de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_dirent_t * const lde = (const fat_lfn_dirent_t *) de;
                u16 chars[FAT_LFN_PART_TOTAL_LEN];
                s32 i, j, offset;

                offset = ((lde->order & 0x1f) - 1) * FAT_LFN_PART_TOTAL_LEN;

                /* Extract this part of the long filename into the "chars" buffer */
                for(j = 0, i = 0; i < FAT_LFN_PART1_LEN;)
                    chars[j++] = LE2N16(lde->name_part1[i++]);

                for(i = 0; i < FAT_LFN_PART2_LEN;)
                    chars[j++] = LE2N16(lde->name_part2[i++]);

                for(i = 0; i < FAT_LFN_PART3_LEN;)
                    chars[j++] = LE2N16(lde->name_part3[i++]);

                for(i = 0; (i < FAT_LFN_PART_TOTAL_LEN) && chars[i]; ++i)
                    lfn[offset + i] = (chars[i] < 0x100) ? (s8) (chars[i] & 0xff) : '?';

                if((offset + i) > lfn_len)
                    lfn_len = offset + i;
            }
            else
            {
                /*
                    Regular directory entry.  If we have already read a long filename, we will use
                    that instead of the short filename in this entry.
                */
                if(lfn_len)
                {
                    /* Long filename has been read */
                    lfn[lfn_len] = '\0';
                    puts(lfn);

                    lfn_len = 0;     /* Reset long filename buffer */
                }
                else
                {
                    /* No long filename has been read - use the short filename from this entry */
                    int i;
                    for(i = 0; i < 11; ++i)
                        putchar(de->file_name[i]);
                    putchar('\n');
                }
            }
        }

        if(dir_end)
            break;

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


/*
    Dump superblock contents for debugging
*/
void fat_debug_dump_superblock(vfs_t * vfs)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;

    printf("FAT superblock for '%s':\n"
           "- total_sectors       = %d\n"
           "- total_clusters      = %d\n"
           "- total_data_sectors  = %d\n"
           "- sectors_per_cluster = %d\n"
           "- bytes per cluster   = %d\n"
           "- sectors_per_fat     = %d\n"
           "- root_dir_sectors    = %d\n"
           "- first_fat_sector    = %d\n"
           "- first_data_sector   = %d\n\n"
           "- total capacity      = %dMB\n"
           "- data capacity       = %dMB\n",
           vfs->dev->name, fs->total_sectors, fs->total_clusters, fs->total_data_sectors,
           fs->sectors_per_cluster, fs->bytes_per_cluster, fs->sectors_per_fat,
           fs->root_dir_sectors, fs->first_fat_sector, fs->first_data_sector,
           (fs->total_sectors * BLOCK_SIZE) >> 20, (fs->total_data_sectors * BLOCK_SIZE) >> 20);
}
