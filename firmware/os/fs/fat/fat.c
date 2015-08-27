/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015


    TODO: validate names when creating new nodes
          many other things
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
    u32 root_dir_sectors;

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
		kfree(fs);
        return EBADSBLK;
    }

    /* Precalculate some useful figures.  TODO: maybe some validation on these numbers? */
    root_dir_sectors = ((LE2N16(bpb.root_entry_count) * sizeof(struct fat_dirent))
                        + (BLOCK_SIZE - 1)) >> LOG_BLOCK_SIZE;

    fs->sectors_per_cluster   = bpb.sectors_per_cluster;
    fs->bytes_per_cluster     = bpb.sectors_per_cluster * LE2N16(bpb.bytes_per_sector);
    fs->sectors_per_fat       = LE2N16(bpb.sectors_per_fat);


    fs->first_fat_sector      = LE2N16(bpb.reserved_sectors);

    fs->first_data_sector     = LE2N16(bpb.reserved_sectors) + (bpb.fats * fs->sectors_per_fat)
                                    + root_dir_sectors;

    fs->root_dir_first_sector = fs->first_data_sector - root_dir_sectors;

    fs->total_sectors         = bpb.sectors_in_volume ? LE2N16(bpb.sectors_in_volume) :
                                    LE2N32(bpb.large_sectors);

    fs->total_data_sectors    = fs->total_sectors
                                    - (LE2N16(bpb.reserved_sectors)
                                       + (bpb.fats * fs->sectors_per_fat) + root_dir_sectors);

    fs->total_clusters        = fs->total_data_sectors / fs->sectors_per_cluster;

    fs->root_dir_clusters     = root_dir_sectors / fs->sectors_per_cluster;

    vfs->data = fs;
    vfs->root_node = FAT_ROOT_NODE;

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

    if((node - FAT_ROOT_NODE) < fs->root_dir_clusters)
    {
        /*
            This node is within the root directory area.  The next node is node+1, unless we have
            reached the end of the root dir nodes.
        */
        if(++*next_node == (fs->root_dir_clusters - FAT_ROOT_NODE))
            *next_node = FAT_CHAIN_TERMINATOR;
    }
    else if(FAT_CHAIN_END(node))
    {
        /* node represents an end-of-chain marker; set next_node to an EOC marker too. */
        *next_node = FAT_CHAIN_TERMINATOR;
    }
    else
    {
        /* Compute offset from start of FAT, in bytes, of the specified node */
        ku32 fat_offset = node * sizeof(fat16_cluster_id);

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
    }

    return SUCCESS;
}


/*
    fat_open_dir() - prepare to iterate over the directory at node
*/
s32 fat_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    fat_dir_ctx_t *dir_ctx;
    u32 bytes_per_cluster;

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

    dir_ctx->buffer_end = ((void *) dir_ctx->buffer) + bytes_per_cluster;
    dir_ctx->node = node;
    dir_ctx->de = dir_ctx->buffer;   /* Point de (addr of current dirent) at start of node buffer */
    dir_ctx->is_root_dir = (node == FAT_ROOT_NODE);

    return SUCCESS;
}


/*
    fat_read_dir() - if name is NULL, read the next entry from a directory and populate dirent with
    its details.  If name is non-NULL, search for an entry matching name and populate the dirent.
*/
s32 fat_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name)
{
    fat_dir_ctx_t *dir_ctx = (fat_dir_ctx_t *) ctx;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    s32 lfn_len, ret;

    while(!FAT_CHAIN_END(dir_ctx->node))
    {
        /* Read the cluster into dir_ctx->buffer (which was alloc'ed by fat_open_dir() */
        ret = fat_read_node(vfs, dir_ctx->node, dir_ctx->buffer);
        if(ret != SUCCESS)
            return ret;

        /* Read the next entry from the node */
        for(lfn_len = 0; dir_ctx->de < dir_ctx->buffer_end; ++dir_ctx->de)
        {
            if(dir_ctx->de->file_name[0] == FAT_DIRENT_END)
                return ENOENT;  /* No more entries in this directory */
            else if((u8) dir_ctx->de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;       /* This entry represents a deleted item */
            else if(dir_ctx->de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_dirent_t * const lde = (const fat_lfn_dirent_t *) dir_ctx->de;
                u16 chars[FAT_LFN_PART_TOTAL_LEN];
                s32 i, j, offset;

                offset = FAT_LFN_PART_NUM(lde->order) * FAT_LFN_PART_TOTAL_LEN;

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

                    /*
                        If the first byte of the name is 0x05, we substitute 0xe5 (which is normally
                        used to indicate an unused/deleted entry), because 0xe5 is a valid char and
                        is otherwise unrepresentable as the first char of a file name.
                    */
                    if(dir_ctx->de->file_name[0] == 0x05)
                        dir_ctx->de->file_name[0] = 0xe5;

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
                    /*
                        Populate the supplied dirent structure.  If dirent is NULL, the caller was
                        checking whether or not the dirent exists, and doesn't care about any
                        details beyond that.
                    */
                    if(dirent != NULL)
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
                            The FAT fs does not conform to the Unix file-permissions system, so we
                            set some defaults here: rwxrwxrwx for writeable files, and r-xr-xr-x
                            for read-only files.
                        */
                        dirent->permissions = (attribs & FAT_FILEATTRIB_READ_ONLY) ?
                                                VFS_PERM_UGORX : VFS_PERM_UGORWX;

                        /* The FAT fs has no concept of file ownership */
                        dirent->gid = 0;
                        dirent->uid = 0;
                    }

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


/*
    fat_create_node() - create a new directory entry

    FIXME: we mustn't allow this fn to extend the root directory, for some reason
*/
s32 fat_create_node(vfs_t *vfs, u32 parent_node, vfs_dirent_t *dirent)
{
    fat_dir_ctx_t *dir_ctx;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    s32 lfn_len, ret;

    /* Open parent node */      /* TODO - remove ref to parent_node */
    if((ret = fat_open_dir(vfs, parent_node, (void **) &dir_ctx)) != SUCCESS)
        return ret;

    /*
        Scan the directory, looking for either an entry with a matching name (and fail if one is
        found) or the end of the directory entries.
    */
    while(!FAT_CHAIN_END(dir_ctx->node))
    {
        /* Read the cluster into dir_ctx->buffer (which was alloc'ed by fat_open_dir() */
        ret = fat_read_node(vfs, dir_ctx->node, dir_ctx->buffer);
        if(ret != SUCCESS)
            return ret;

        /* Read the next entry from the node */
        for(lfn_len = 0; dir_ctx->de < dir_ctx->buffer_end; ++dir_ctx->de)
        {
            if(dir_ctx->de->file_name[0] == FAT_DIRENT_END)
            {
                /* Found the end of the entries in this directory */
                /* Calculate the amount of space needed for the new entry */
                ku32 bytes_needed =
                    (sizeof(fat_lfn_dirent_t) *
                        ((strlen(dirent->name) + (FAT_LFN_PART_TOTAL_LEN - 1)) /
                            FAT_LFN_PART_TOTAL_LEN)) +
                    sizeof(fat_dirent_t);

                /* Is there enough space in this cluster? */
                if(bytes_needed > (dir_ctx->buffer_end - dir_ctx->de))
                {
                    /* No: find a free cluster and allocate it before proceeding */
                    /* TODO */
                    /* maybe TODO: handle root dir special case? can't be extended */
                    /* If no free clusters, call fat_close_dir() and return ENOSPC. */
                }

                /* No: extend the chain, write as much as poss into the current clus, write the
                    balance into the new clus */
                /*
                    TODO
                        append LFN entries for new node
                        append short-filename entry, including other dirent data
                        type(new_dirent)==dir?
                            create new empty dir structure (allocate one cluster, zero it)
                            set size=0
                            create "." and ".." entries
                            ...more here
                        type(new_dirent)==file?
                            set size=0
                */
            }
            else if((u8) dir_ctx->de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;       /* This entry represents a deleted item */
            else if(dir_ctx->de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_dirent_t * const lde = (const fat_lfn_dirent_t *) dir_ctx->de;
                u16 chars[FAT_LFN_PART_TOTAL_LEN];
                s32 i, j, offset;

                offset = FAT_LFN_PART_NUM(lde->order) * FAT_LFN_PART_TOTAL_LEN;

                /* Extract this part of the long filename into the "chars" buffer */
                for(j = 0, i = 0; i < FAT_LFN_PART1_LEN;)
                    chars[j++] = LE2N16(lde->name_part1[i++]);

                for(i = 0; i < FAT_LFN_PART2_LEN;)
                    chars[j++] = LE2N16(lde->name_part2[i++]);

                for(i = 0; i < FAT_LFN_PART3_LEN;)
                    chars[j++] = LE2N16(lde->name_part3[i++]);

                /* Copy any representable ASCII-representable chars into lfn[] */
                /* The FAT spec requires that all non-representable chars are translated to '_' */
                for(i = 0; (i < FAT_LFN_PART_TOTAL_LEN) && chars[i]; ++i)
                    lfn[offset + i] = (chars[i] < 0x80) ? (s8) (chars[i] & 0xff) : '_';

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

                    /*
                        If the first byte of the name is 0x05, we substitute 0xe5 (which is normally
                        used to indicate an unused/deleted entry), because 0xe5 is a valid char and
                        is otherwise unrepresentable as the first char of a file name.
                    */
                    if(dir_ctx->de->file_name[0] == 0x05)
                        dir_ctx->de->file_name[0] = 0xe5;

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
                    its name matches dirent->name (i.e. the name requested for the new node), fail.
                */
                if(!strcasecmp(dirent->name, lfn))
                {
                    fat_close_dir(vfs, dir_ctx);
                    return EEXIST;
                }
            }
        }

        /* Reached the end of the node without finding a complete entry. */
        /* Find the next node in the chain */
        ret = fat_get_next_node(vfs, dir_ctx->node, &dir_ctx->node);
        if(ret != SUCCESS)
        {
            fat_close_dir(vfs, dir_ctx);
            return ret;
        }
    }

    fat_close_dir(vfs, dir_ctx);

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

    return ENOSPC;   /* No free nodes found */
}


/*
    fat_lfn_checksum() - compute a one-byte checksum from an 11-character raw short filename string,
    e.g. "FOO     BAR" (representing the name "FOO.BAR")
*/
u8 fat_lfn_checksum(u8 *short_name)
{
    u16 x;
    u8 sum;

    for(sum = 0, x = FAT_FILENAME_LEN + FAT_FILEEXT_LEN; x; --x)
        sum = ((sum & 1) ? 0x80 : 0x00) + (sum >> 1) + *short_name++;

    return sum;
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
           "- root_dir_clusters   = %d\n"
           "- first_fat_sector    = %d\n"
           "- first_data_sector   = %d\n\n"
           "- total capacity      = %dMB\n"
           "- data capacity       = %dMB\n",
           vfs->dev->name, fs->total_sectors, fs->total_clusters, fs->total_data_sectors,
           fs->sectors_per_cluster, fs->bytes_per_cluster, fs->sectors_per_fat,
           fs->root_dir_clusters, fs->first_fat_sector, fs->first_data_sector,
           (fs->total_sectors * BLOCK_SIZE) >> 20, (fs->total_data_sectors * BLOCK_SIZE) >> 20);
}
