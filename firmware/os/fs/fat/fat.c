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
    .open_dir = fat_open_dir,
    .read_dir = fat_read_dir,
    .close_dir = fat_close_dir,
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
/*
    puts("\n$ ls /");

    void *ctx;
    vfs_dirent_t dirent;

    ret = fat_open_dir(vfs, 15, &ctx);
    if(ret != SUCCESS)
    {
        printf("fat_open_dir() failed: %s\n", kstrerror(ret));
        return ret;
    }

    while((ret = fat_read_dir(vfs, ctx, &dirent, NULL)) == SUCCESS)
    {
        printf("%c %9d %9d %s\n", (dirent.type == FSNODE_TYPE_DIR) ? 'd' : 'f', dirent.first_node,
               dirent.size, dirent.name);
    }

    if(ret != ENOENT)
        printf("fat_read_dir() failed with %s\n", kstrerror(ret));

    fat_close_dir(vfs, ctx);
*/
    return SUCCESS;
}


s32 fat_umount(vfs_t *vfs)
{
    /* TODO: (both of these tasks may not be this module's responsibility):
        - flush all dirty sectors
        - ensure no open file handles exist on this fs
        - (maybe) duplicate the main FAT into the secondary FAT, if present
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


/*
    fat_open_dir() - prepare to iterate over the directory at node
*/
s32 fat_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    fat_dir_ctx_t *dir_ctx;
    u32 bytes_per_cluster;
    s32 ret;

    /* Allocate space to hold a directory context */
    *ctx = kmalloc(sizeof(fat_dir_ctx_t));
    if(!*ctx)
        return ENOMEM;

    dir_ctx = (fat_dir_ctx_t *) *ctx;

    /* Allocate space within the directory context to hold a node */
    bytes_per_cluster = ((fat_fs_t *) vfs->data)->bytes_per_cluster;
    dir_ctx->buffer = (fat_dirent_t *) kmalloc(bytes_per_cluster);
    if(!dir_ctx->buffer)
    {
        kfree(*ctx);
        return ENOMEM;
    }

    ret = fat_read_node(vfs, node, dir_ctx->buffer);
    if(ret != SUCCESS)
    {
        kfree(dir_ctx->buffer);
        kfree(*ctx);
        return ret;
    }

    dir_ctx->buffer_end = ((void *) dir_ctx->buffer) + bytes_per_cluster;
    dir_ctx->node = node;
    dir_ctx->de = dir_ctx->buffer;   /* Point de (addr of current dirent) at start of node buffer */

    return SUCCESS;
}


/*
    fat_read_dir() - if name is NULL, read the next entry from a directory and populate dirent with
    its details.  If name is non-NULL, search for an entry matching name and populate the dirent.
*/
s32 fat_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, const s8 * const name)
{
    fat_dir_ctx_t *dir_ctx = (fat_dir_ctx_t *) ctx;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    s32 lfn_len, ret;

    while(!FAT_CHAIN_END(dir_ctx->node))
    {
        /* Read the next entry from the node */
        for(lfn_len = 0; dir_ctx->de < dir_ctx->buffer_end; ++dir_ctx->de)
        {
            if(dir_ctx->de->file_name[0] == 0x00)
                return ENOENT;  /* No more entries in this directory */
            else if((u8) dir_ctx->de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;
            else if(dir_ctx->de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_dirent_t * const lde = (const fat_lfn_dirent_t *) dir_ctx->de;
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

                /* Copy any representable ASCII-representable chars into lfn[] */
                for(i = 0; (i < FAT_LFN_PART_TOTAL_LEN) && chars[i]; ++i)
                    lfn[offset + i] = (chars[i] < 0x80) ? (s8) (chars[i] & 0xff) : '?';

                if((offset + i) > lfn_len)
                    lfn_len = offset + i;
            }
            else if(dir_ctx->de->attribs & FAT_FILEATTRIB_VOLUME_ID)
                continue;       /* Ignore the volume ID */
            else
            {
                /*
                    Regular directory entry.  If we have already read a long filename, we will use
                    that instead of the short filename in this entry.
                */
                if(!lfn_len)
                {
                    /*
                        We didn't encounter a long filename, so write the regular (short) filename
                        into the long filename buffer.
                    */
                    s32 len;

                    strn_trim_cpy(lfn, dir_ctx->de->file_name, FAT_FILENAME_LEN);
                    len = strlen(lfn);

                    lfn[len++] = '.';
                    strn_trim_cpy(lfn + len, dir_ctx->de->file_name + FAT_FILENAME_LEN,
                                  FAT_FILEEXT_LEN);

                    /* If no file extension is present, trim the trailing '.' char */
                    if(strlen(lfn + len - 1) == 1)
                        lfn[len - 1] = '\0';
                }
                else
                {
                    /* Long filename has been read */
                    lfn[lfn_len] = '\0';
                    lfn_len = 0;     /* Reset long filename buffer */
                }

                /*
                    At this point we have a complete directory entry, with a filename in lfn.  If
                    the "name" arg is NULL, populate a dirent and return it; otherwise, compare the
                    entry's name with "name" (case-insensitive, because FAT) and return a dirent if
                    there is a match.  If there is no match, continue searching the directory.
                */
                if((name == NULL) || !strcasecmp(name, lfn))
                {
                    ku16 attribs = dir_ctx->de->attribs;
                    u16 flags = 0;

                    dirent->vfs = vfs;

                    dirent->atime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->adate), 0);
                    dirent->ctime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->cdate),
                                                                LE2N16(dir_ctx->de->ctime));
                    dirent->mtime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->mdate),
                                                                LE2N16(dir_ctx->de->mtime));

                    dirent->first_node = (LE2N16(dir_ctx->de->first_cluster_high) << 16)
                                            | LE2N16(dir_ctx->de->first_cluster_low);

                    dirent->type = (attribs & FAT_FILEATTRIB_DIRECTORY) ?
                                        FSNODE_TYPE_DIR : FSNODE_TYPE_FILE;

                    if(attribs & FAT_FILEATTRIB_HIDDEN)
                        flags |= VFS_FLAG_HIDDEN;
                    if(attribs & FAT_FILEATTRIB_SYSTEM)
                        flags |= VFS_FLAG_SYSTEM;
                    if(attribs & FAT_FILEATTRIB_ARCHIVE)
                        flags |= VFS_FLAG_ARCHIVE;

                    dirent->flags = flags;

                    strcpy(dirent->name, lfn);
                    dirent->size = LE2N32(dir_ctx->de->size);

                    /*
                        None of the following three items (permissions, uid, gid) have any meaning
                        in the FAT filesystem.
                    */
                    dirent->permissions = (attribs & FAT_FILEATTRIB_READ_ONLY) ?
                                            VFS_PERM_UGORX : VFS_PERM_UGORWX;
                    dirent->gid = 0;
                    dirent->uid = 0;

                    ++dir_ctx->de;
                    return SUCCESS;
                }
            }
        }

        /* Reached the end of the node without finding a complete entry. */
        /* Find the next node in the chain */
        ret = fat_get_next_node(vfs, dir_ctx->node, &dir_ctx->node);
        if(ret != SUCCESS)
            return ret;
    }

    return ENOENT;  /* Reached the end of the chain */
}


/*
    fat_close_dir() - clean up after iterating over a directory
*/
s32 fat_close_dir(vfs_t *vfs, void *ctx)
{
    kfree(((fat_dir_ctx_t *) ctx)->buffer);
    kfree(ctx);
    return SUCCESS;
}


s32 fat_stat(vfs_t *vfs, fs_stat_t *st)
{

    return 0;
}


/*
    fat_find_free_node() - find the first free node in the FAT.
*/
s32 fat_find_free_node(vfs_t *vfs, u32 *node)
{
    u32 u;
    u16 sector[BLOCK_SIZE >> 1];
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;

    for(u = 0; u < fs->sectors_per_fat; ++u)
    {
        u32 v;

        /* Read sector */
        u32 ret = device_read(vfs->dev, fs->first_fat_sector + u, 1, &sector);
        if(ret != SUCCESS)
            return ret;

        /* HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK */
        fat_hack_sector_swap_bytes(&sector, 1);

        for(v = 0; v < ARRAY_COUNT(sector); ++v)
        {
            if(!sector[v])
            {
                *node = v + (u * ARRAY_COUNT(sector));
                return SUCCESS;
            }
        }
    }

    return ENOENT;   /* No free nodes found */
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
