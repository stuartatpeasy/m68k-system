/*
    FAT16 file system support

    Part of ayumos


    (c) Stuart Wallace, July 2015


    TODO: validate names when creating new nodes
          many other things
*/

#ifdef WITH_FS_FAT

#include <kernel/fs/fat/fat.h>
#include <kernel/include/device/block.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/error.h>
#include <kernel/include/types.h>
#include <kernel/util/kutil.h>
#include <klibc/include/strings.h>


struct fat_bpb_block
{
    u8  jmp[3];
    s8  oem_id[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fats;
    u16 root_entry_count;
    u16 sectors_in_volume;
    u8  media_descriptor_type;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 large_sectors;
    u8  drive_number;
    u8  winnt_flags;
    u8  signature;
    u32 volume_serial_number;
    s8  volume_label[11];
    s8  system_identifier[8];
    u8  boot_code[448];
    u16 partition_signature;
} __attribute__((packed));  /* 512 bytes */


/*
    FAT directory entry
*/
#define FAT_FILENAME_LEN        (8)     /* Note: long filenames are dealt with separately */
#define FAT_FILEEXT_LEN         (3)     /* Note: long filenames are dealt with separately */

struct fat_node
{
    s8  file_name[11];
    u8  attribs;
    u8  winnt_reserved;
    u8  ctime_tenths;
    u16 ctime;
    u16 cdate;
    u16 adate;
    u16 first_cluster_high;
    u16 mtime;
    u16 mdate;
    u16 first_cluster_low;
    u32 size;
} __attribute__((packed));  /* 32 bytes */

typedef struct fat_node fat_node_t;


/*
    FAT long filename directory entry
*/
#define FAT_LFN_PART1_LEN       (5)
#define FAT_LFN_PART2_LEN       (6)
#define FAT_LFN_PART3_LEN       (2)
#define FAT_LFN_PART_TOTAL_LEN  (FAT_LFN_PART1_LEN + FAT_LFN_PART2_LEN + FAT_LFN_PART3_LEN)

struct fat_lfn_node
{
    u8 order;
    u16 name_part1[FAT_LFN_PART1_LEN];      /* UTF-16 */
    u8 attribs;
    u8 type;
    u8 checksum;
    u16 name_part2[FAT_LFN_PART2_LEN];      /* UTF-16 */
    u16 reserved;
    u16 name_part3[FAT_LFN_PART3_LEN];      /* UTF-16 */
} __attribute__((packed));

typedef struct fat_lfn_node fat_lfn_node_t;


/*
    This struct contains various useful numbers, computed at mount time.  A struct fat_fs will be
    allocated and filled in fat_mount(), and stored in vfs->data.  It is then deallocated in
    fat_unmount().
*/
struct fat_fs
{
    u32 total_sectors;
    u32 total_data_sectors;
    u32 first_data_sector;
    u32 first_fat_sector;
    u32 root_dir_first_sector;
    u16 total_clusters;
    u16 root_dir_clusters;
    u16 sectors_per_fat;
    u16 sectors_per_cluster;
    u16 bytes_per_cluster;
    u16 last_free_cluster;
};

typedef struct fat_fs fat_fs_t;


struct fat_dir_ctx
{
    fat_node_t *buffer;
    fat_node_t *buffer_end;
    u32 block;
    fat_node_t *de;
    s8 is_root_dir;
};

typedef struct fat_dir_ctx fat_dir_ctx_t;


typedef u16 fat16_cluster_id;

#define FAT_ROOT_BLOCK              (0)             /* Only valid in FAT12/FAT16 */

#define FAT_FIRST_DATA_CLUSTER      (2)

/* Constants used during superblock validation */
#define FAT_PARTITION_SIG           (0xaa55)
#define FAT_JMP_BYTE0               (0xeb)
#define FAT_JMP_BYTE2               (0x90)

/* File attribute constants */
#define FAT_FILEATTRIB_READ_ONLY    (0x01)
#define FAT_FILEATTRIB_HIDDEN       (0x02)
#define FAT_FILEATTRIB_SYSTEM       (0x04)
#define FAT_FILEATTRIB_VOLUME_ID    (0x08)
#define FAT_FILEATTRIB_DIRECTORY    (0x10)
#define FAT_FILEATTRIB_ARCHIVE      (0x20)

/* Long filename indicator */
#define FAT_FILEATTRIB_LFN          (FAT_FILEATTRIB_READ_ONLY | \
                                     FAT_FILEATTRIB_HIDDEN | \
                                     FAT_FILEATTRIB_SYSTEM | \
                                     FAT_FILEATTRIB_VOLUME_ID)

#define FAT_LFN_MAX_LEN             (255)
#define FAT_LFN_ORDER_MASK          (0x1f)  /* AND this with LFN order byte to get LFN order + 1 */

#define FAT_LFN_PART_NUM(order_byte) \
    (((order_byte) & FAT_LFN_ORDER_MASK) - 1)

#define FAT_CHAIN_TERMINATOR        (0xfff7)
#define FAT_CHAIN_END(x)            ((x) >= FAT_CHAIN_TERMINATOR)

#define FAT_DIRENT_END              (0x00)  /* End-of-directory-entries marker  */
#define FAT_DIRENT_UNUSED           (0xe5)  /* Deleted directory entry marker   */

/* FAT-format date/time extraction/conversion macros */
#define FAT_YEAR_FROM_DATE(d)       ((((d) & 0xFE00) >> 9) + 1980)
#define FAT_MONTH_FROM_DATE(d)      (((d) & 0x01E0) >> 5)
#define FAT_DAY_FROM_DATE(d)        ((d) & 0x001F)

#define FAT_HOUR_FROM_TIME(t)       (((t) & 0xF800) >> 11)
#define FAT_MINUTE_FROM_TIME(t)     (((t) & 0x07E0) >> 5)
#define FAT_SECOND_FROM_TIME(t)     (((t) & 0x001F) << 1)

#define FAT_DATE_TO_RTC_DATE(fdate, rdate)                  \
{                                                           \
    (rdate).year = FAT_YEAR_FROM_DATE(fdate);               \
    (rdate).month = FAT_MONTH_FROM_DATE(fdate);             \
    (rdate).day = FAT_DAY_FROM_DATE(fdate);                 \
}

#define FAT_TIME_TO_RTC_DATE(ftime, rdate)                  \
{                                                           \
    (rdate).hour = FAT_HOUR_FROM_TIME(ftime);               \
    (rdate).minute = FAT_MINUTE_FROM_TIME(ftime);           \
    (rdate).second = FAT_SECOND_FROM_TIME(ftime);           \
}

#define FAT_DATETIME_TO_RTC_DATETIME(fdate, ftime, rdate)   \
{                                                           \
    FAT_DATE_TO_RTC_DATE(fdate, rdate);                     \
    FAT_TIME_TO_RTC_DATE(ftime, rdate);                     \
}

#define FAT_DATETIME_TO_TIMESTAMP(fdate, ftime)             \
    __extension__ ({                                        \
        struct rtc_time tm;                                 \
        s32 ts;                                             \
        FAT_DATE_TO_RTC_DATE(fdate, tm);                    \
        FAT_TIME_TO_RTC_DATE(ftime, tm);                    \
        rtc_time_to_timestamp(&tm, &ts);                    \
        ts;                                                 \
    })

#define RTC_DATE_TO_FAT_DATE(rdate)                         \
    __extension__ ({                                        \
        (u16) (((rdate)->year - 1980) << 9) |               \
                ((rdate)->month << 5) |                     \
                ((rdate)->day);                             \
    })

#define RTC_DATE_TO_FAT_TIME(rdate)                         \
    __extension__ ({                                        \
        (u16) ((rdate)->hour << 11) |                       \
                ((rdate)->minute << 5) |                    \
                ((rdate)->second >> 1);                     \
    })


s32 fat_init();
s32 fat_mount(vfs_t *vfs);
s32 fat_unmount(vfs_t *vfs);
s32 fat_get_root_node(vfs_t *vfs, fs_node_t **node);
s32 fat_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 fat_read_dir(vfs_t *vfs, void *ctx, ks8* const name, fs_node_t *node);
s32 fat_close_dir(vfs_t *vfs, void *ctx);
s32 fat_read(vfs_t * const vfs, fs_node_t * const node, void * const buffer, u32 offset, ks32 count);
s32 fat_write(vfs_t * const vfs, fs_node_t * const node, const void * const buffer, u32 offset,
              ks32 count);
s32 fat_reallocate(vfs_t * const vfs, fs_node_t * const node, ks32 new_len);
s32 fat_stat(vfs_t *vfs, fs_stat_t *st);

s32 fat_read_cluster(vfs_t * const vfs, ku32 cluster, void * const buffer);
s32 fat_read_cluster_partial(vfs_t * const vfs, ku32 cluster, void *buffer, ku16 offset,
                             ku16 count);
s32 fat_write_cluster(vfs_t * const vfs, ku32 cluster, const void * const buffer);
s32 fat_write_cluster_partial(vfs_t *vfs, u32 cluster, const void *buffer, ku16 offset, ku16 count);
s32 fat_create_node(vfs_t *vfs, u32 parent_block, fs_node_t *node);
s32 fat_get_next_cluster(vfs_t *vfs, u32 node, u32 *next_node);
s32 fat_alloc_cluster(vfs_t *vfs);
s32 fat_generate_basis_name(s8 * lfn, u32 tailnum, char * const basis_name);
u8 fat_lfn_checksum(u8 *short_name);

#ifdef FAT_DEBUG
void fat_debug_dump_superblock(vfs_t *vfs);
#else
#define fat_debug_dump_superblock(dummy) ((void) dummy)
#endif


vfs_driver_t g_fat_ops =
{
    .name           = "fat",
    .init           = fat_init,
    .mount          = fat_mount,
    .unmount        = fat_unmount,
    .get_root_node  = fat_get_root_node,
    .open_dir       = fat_open_dir,
    .read_dir       = fat_read_dir,
    .close_dir      = fat_close_dir,
    .read           = fat_read,
    .write          = fat_write,
    .reallocate     = fat_reallocate,
    .stat           = fat_stat
};


s32 fat_init()
{
    /* Nothing to do here */
    return SUCCESS;
}


/*
    fat_mount() - attempt to mount a FAT file system.
*/
s32 fat_mount(vfs_t *vfs)
{
    struct fat_bpb_block bpb;
    struct fat_fs *fs;
    s32 ret;
    u32 root_dir_sectors;

    /* Read first sector */
    ret = block_read(vfs->dev, 0, &bpb);
    if(ret != SUCCESS)
        return ret;

    /* Validate jump entry */
    if((bpb.jmp[0] != 0xeb) || (bpb.jmp[2] != 0x90))
    {
        printf("%s: bad FAT superblock: incorrect jump bytes: expected 0xeb 0xxx 0x90, "
               "read 0x%02x 0x%02x 0x%02x\n", vfs->dev->name, bpb.jmp[0], bpb.jmp[1], bpb.jmp[2]);
        return -EBADSBLK;
    }

    /* Validate partition signature */
    if(LE2N16(bpb.partition_signature) != FAT_PARTITION_SIG)
    {
        printf("%s: bad FAT superblock: incorrect partition signature: expected 0x%04x, "
               "read 0x%04x\n", vfs->dev->name, FAT_PARTITION_SIG, LE2N16(bpb.partition_signature));
        return -EBADSBLK;
    }

    fs = slab_alloc(sizeof(struct fat_fs));
    if(!fs)
        return -ENOMEM;

    /* FIXME - use BLOCK_SIZE everywhere?  e.g. retire ATA_SECTOR_SIZE, etc. */
    if(LE2N16(bpb.bytes_per_sector) != BLOCK_SIZE)
    {
        printf("%s: bad FAT sector size %d; can only handle %d-byte sectors\n",
               vfs->dev->name, LE2N16(bpb.bytes_per_sector), BLOCK_SIZE);
        slab_free(fs);
        return -EBADSBLK;
    }

    /* Precalculate some useful figures.  TODO: maybe some validation on these numbers? */
    root_dir_sectors = ((LE2N16(bpb.root_entry_count) * sizeof(fat_node_t))
                        + (BLOCK_SIZE - 1)) >> LOG_BLOCK_SIZE;

    fs->sectors_per_cluster     = bpb.sectors_per_cluster;
    fs->bytes_per_cluster       = bpb.sectors_per_cluster * LE2N16(bpb.bytes_per_sector);
    fs->sectors_per_fat         = LE2N16(bpb.sectors_per_fat);


    fs->first_fat_sector        = LE2N16(bpb.reserved_sectors);

    fs->first_data_sector       = LE2N16(bpb.reserved_sectors) + (bpb.fats * fs->sectors_per_fat)
                                  + root_dir_sectors;

    fs->root_dir_first_sector   = fs->first_data_sector - root_dir_sectors;

    fs->total_sectors           = bpb.sectors_in_volume ? LE2N16(bpb.sectors_in_volume) :
                                  LE2N32(bpb.large_sectors);

    fs->total_data_sectors      = fs->total_sectors - (LE2N16(bpb.reserved_sectors)
                                  + (bpb.fats * fs->sectors_per_fat) + root_dir_sectors);

    fs->total_clusters          = fs->total_data_sectors / fs->sectors_per_cluster;

    fs->root_dir_clusters       = root_dir_sectors / fs->sectors_per_cluster;

    fs->last_free_cluster       = FAT_FIRST_DATA_CLUSTER;

    vfs->data = fs;
    vfs->root_block = FAT_ROOT_BLOCK;

    return SUCCESS;
}


s32 fat_unmount(vfs_t *vfs)
{
    /* TODO: (both of these tasks may not be this module's responsibility):
        - flush all dirty sectors
        - ensure no open file handles exist on this fs
        - (maybe) duplicate the main FAT into the secondary FAT, if present
    */
    slab_free(vfs->data);

    return SUCCESS;
}


/*
    fat_read_cluster() - read cluster <cluster> into <buffer>.
*/
s32 fat_read_cluster(vfs_t * const vfs, ku32 cluster, void * const buffer)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u32 block;

    if(cluster >= fs->total_clusters)
        return -EINVAL;

    block = ((cluster - FAT_FIRST_DATA_CLUSTER) * fs->sectors_per_cluster) + fs->first_data_sector;

    return block_read_multi(vfs->dev, block, fs->sectors_per_cluster, buffer);
}


/*
    fat_read_cluster_partial() - read <count> blocks, starting from block <offset>, from <cluster>
    into <buffer>.
*/
s32 fat_read_cluster_partial(vfs_t * const vfs, ku32 cluster, void *buffer, ku16 offset, ku16 count)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u32 block;

    if((cluster >= fs->total_clusters) || ((offset + count) > fs->sectors_per_cluster))
        return -EINVAL;

    block = ((cluster - FAT_FIRST_DATA_CLUSTER) * fs->sectors_per_cluster)
            + fs->first_data_sector + offset;

    return block_read_multi(vfs->dev, block, count, buffer);
}


/*
    fat_write_cluster() - write <buffer> into cluster <cluster>.
*/
s32 fat_write_cluster(vfs_t * const vfs, ku32 cluster, const void * const buffer)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u32 block;

    if(cluster > fs->total_clusters)
        return -EINVAL;

    block = ((cluster - FAT_FIRST_DATA_CLUSTER) * fs->sectors_per_cluster) + fs->first_data_sector;

    return block_write_multi(vfs->dev, block, fs->sectors_per_cluster, buffer);
}


/*
    fat_write_cluster_partial() - write <count> blocks of data from <buffer> to <cluster>, starting
    <offset> blocks from the beginning of the cluster.
*/
s32 fat_write_cluster_partial(vfs_t *vfs, u32 cluster, const void *buffer, ku16 offset, ku16 count)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u32 block;

    if((offset + count) > fs->sectors_per_cluster)
        return -EINVAL;

    block = ((cluster - FAT_FIRST_DATA_CLUSTER) * fs->sectors_per_cluster)
            + fs->first_data_sector + offset;

    return block_write_multi(vfs->dev, block, count, buffer);
}


/*
    fat_get_next_cluster() - given a cluster, find the next cluster in the chain.
*/
s32 fat_get_next_cluster(vfs_t *vfs, u32 node, u32 *next_node)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    fat16_cluster_id sector[BLOCK_SIZE / sizeof(fat16_cluster_id)];

    if((node - FAT_ROOT_BLOCK) < fs->root_dir_clusters)
    {
        /*
            This node is within the root directory area.  The next node is node+1, unless we have
            reached the end of the root dir nodes.
        */
        if(++*next_node == (fs->root_dir_clusters - FAT_ROOT_BLOCK))
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
        u32 ret = block_read(vfs->dev, fat_sector, &sector);
        if(ret != SUCCESS)
            return ret;

        *next_node = LE2N16(sector[ent_offset]);
    }

    return SUCCESS;
}


/*
    fat_get_root_node() - populate a fs_node_t with details of the root directory
*/
s32 fat_get_root_node(vfs_t *vfs, fs_node_t **node)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    fs_node_t *root_node;
    s32 ret;

    ret = fs_node_alloc(&root_node);
    if(ret != SUCCESS)
        return ret;

    /* Zero out the node struct - that way we only have to set nonzero fields */
    bzero(root_node, sizeof(fs_node_t));

    /*
        Note: FAT does not store ctime/mtime/atime for the whole fs, so they are left as zeroes in
        the root node.  UID and GID are also not stored (or applicable), so these fields are also
        left as zero.
    */

    ret = fs_node_set_name(root_node, ROOT_DIR);
    if(ret != SUCCESS)
    {
        fs_node_free(root_node);
        return ret;
    }

    root_node->type = FSNODE_TYPE_DIR;
    root_node->permissions = FS_PERM_UGORWX;
    root_node->size = fs->root_dir_clusters * fs->bytes_per_cluster;
    root_node->first_block = FAT_ROOT_BLOCK;

    *node = root_node;

    return SUCCESS;
}


/*
    fat_open_dir() - prepare to iterate over the directory at <block>
*/
s32 fat_open_dir(vfs_t *vfs, u32 block, void **ctx)
{
    fat_dir_ctx_t *dir_ctx;
    fat_fs_t * const fs = (fat_fs_t *) vfs->data;
    u32 bytes_per_cluster;
    s32 ret;

    /* Allocate space to hold a directory context */
    dir_ctx = (fat_dir_ctx_t *) slab_alloc(sizeof(fat_dir_ctx_t));
    if(!dir_ctx)
        return -ENOMEM;

    /* Allocate space within the directory context to hold a block */
    bytes_per_cluster = fs->bytes_per_cluster;
    dir_ctx->buffer = (fat_node_t *) kmalloc(bytes_per_cluster);
    if(!dir_ctx->buffer)
    {
        slab_free(dir_ctx);
        return -ENOMEM;
    }

    /* Read the cluster into dir_ctx->buffer */
    ret = fat_read_cluster(vfs, block, dir_ctx->buffer);
    if(ret != SUCCESS)
    {
        kfree(dir_ctx->buffer);
        slab_free(dir_ctx);
        return ret;
    }

    dir_ctx->buffer_end = ((void *) dir_ctx->buffer) + bytes_per_cluster;
    dir_ctx->block = block;
    dir_ctx->de = dir_ctx->buffer;   /* Point de (addr of current node) at start of node buffer */          /////////////////////////////////////////////////////
    dir_ctx->is_root_dir = (block== FAT_ROOT_BLOCK);

    *ctx = dir_ctx;

    return SUCCESS;
}


/*
    fat_read_dir() - if name is NULL, read the next entry from a directory and populate node with
    its details.  If name is non-NULL, search for an entry matching name and populate the node.
*/
s32 fat_read_dir(vfs_t *vfs, void *ctx, ks8 * const name, fs_node_t *node)
{
    fat_dir_ctx_t *dir_ctx = (fat_dir_ctx_t *) ctx;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    s32 lfn_len, ret;

    while(!FAT_CHAIN_END(dir_ctx->block))
    {
        /* Read the next entry from the node */
        for(lfn_len = 0; dir_ctx->de < dir_ctx->buffer_end; ++dir_ctx->de)
        {
            if(dir_ctx->de->file_name[0] == FAT_DIRENT_END)
                return -ENOENT;     /* No more entries in this directory */
            else if((u8) dir_ctx->de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;           /* This entry represents a deleted item */
            else if(dir_ctx->de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_node_t * const lde = (const fat_lfn_node_t *) dir_ctx->de;
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
                    the "name" arg is NULL, populate a node and return it; otherwise, compare the
                    entry's name with "name" (case-insensitive, because FAT) and return a node if
                    there is a match.  If there is no match, continue searching the directory.
                */
                if((name == NULL) || !strcasecmp(name, lfn))
                {
                    /*
                        Populate the supplied node structure.  If node is NULL, the caller was
                        checking whether or not the node exists, and doesn't care about any details
                        beyond that.
                    */
                    if(node != NULL)
                    {
                        ku16 attribs = dir_ctx->de->attribs;
                        u16 flags = 0;

                        node->atime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->adate), 0);
                        node->ctime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->cdate),
                                                                    LE2N16(dir_ctx->de->ctime));
                        node->mtime = FAT_DATETIME_TO_TIMESTAMP(LE2N16(dir_ctx->de->mdate),
                                                                    LE2N16(dir_ctx->de->mtime));

                        node->first_block = (LE2N16(dir_ctx->de->first_cluster_high) << 16)
                                                | LE2N16(dir_ctx->de->first_cluster_low);

                        node->type = (attribs & FAT_FILEATTRIB_DIRECTORY) ?
                                            FSNODE_TYPE_DIR : FSNODE_TYPE_FILE;

                        if(attribs & FAT_FILEATTRIB_HIDDEN)
                            flags |= FS_FLAG_HIDDEN;
                        if(attribs & FAT_FILEATTRIB_SYSTEM)
                            flags |= FS_FLAG_SYSTEM;
                        if(attribs & FAT_FILEATTRIB_ARCHIVE)
                            flags |= FS_FLAG_ARCHIVE;

                        node->flags = flags;

                        ret = fs_node_set_name(node, lfn);
                        if(ret != SUCCESS)
                            return ret;

                        node->size = LE2N32(dir_ctx->de->size);

                        /*
                            The FAT fs does not conform to the Unix file-permissions system, so we
                            set some defaults here: rwxrwxrwx for writeable files, and r-xr-xr-x
                            for read-only files.
                        */
                        node->permissions = (attribs & FAT_FILEATTRIB_READ_ONLY) ?
                                                FS_PERM_UGORX : FS_PERM_UGORWX;

                        /* The FAT fs has no concept of file ownership */
                        node->gid = 0;
                        node->uid = 0;
                    }

                    ++dir_ctx->de;
                    return SUCCESS;
                }
            }
        }

        /* Reached the end of the node without finding a complete entry. */
        /* Find the next node in the chain */
        ret = fat_get_next_cluster(vfs, dir_ctx->block, &dir_ctx->block);
        if(ret != SUCCESS)
            return ret;

        if(!FAT_CHAIN_END(dir_ctx->block))
        {
            /* Read the next cluster into dir_ctx->buffer (which was alloc'ed by fat_open_dir() */
            ret = fat_read_cluster(vfs, dir_ctx->block, dir_ctx->buffer);
            if(ret != SUCCESS)
                return ret;
        }
    }

    return -ENOENT;     /* Reached the end of the chain */
}


/*
    fat_close_dir() - clean up after iterating over a directory
*/
s32 fat_close_dir(vfs_t *vfs, void *ctx)
{
    UNUSED(vfs);

    kfree(((fat_dir_ctx_t *) ctx)->buffer);
    slab_free(ctx);

    return SUCCESS;
}


/*
    fat_read() - read <count> blocks, starting from <offset>, into <buffer> from the file indicated
    by <node>.
*/
s32 fat_read(vfs_t * const vfs, fs_node_t * const node, void * const buffer, u32 offset, ks32 count)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u8 *buffer_;
    u32 cluster, remaining;
    s32 ret;

    if(count < 0)
        return -EINVAL;

    buffer_ = (u8 *) buffer;
    cluster = node->first_block;
    remaining = count;

    /* Find the first cluster containing the blocks to be read */
    for(cluster = node->first_block; offset >= fs->sectors_per_cluster;
        offset -= fs->sectors_per_cluster)
    {
        ret = fat_get_next_cluster(vfs, cluster, &cluster);
        if(ret != SUCCESS)
            return ret;

        if(FAT_CHAIN_END(cluster))
            return -EINVAL;
    }

    /* Read the appropriate part of this cluster into the buffer */
    ret = fat_read_cluster_partial(vfs, cluster, buffer_, offset, fs->sectors_per_cluster - offset);
    if(ret != SUCCESS)
        return ret;

    remaining -= fs->sectors_per_cluster - offset;
    buffer_ += BLOCK_SIZE * (fs->sectors_per_cluster - offset);

    /* Copy additional clusters, as needed */
    for(; remaining >= fs->sectors_per_cluster; remaining -= fs->sectors_per_cluster)
    {
        ret = fat_get_next_cluster(vfs, cluster, &cluster);
        if(ret != SUCCESS)
            return ret;

        if(FAT_CHAIN_END(cluster))
            return count - remaining;           /* Partial read */

        ret = fat_read_cluster(vfs, cluster, buffer_);
        if(ret != SUCCESS)
            return ret;

        buffer_ += BLOCK_SIZE * fs->sectors_per_cluster;
    }

    /* Final cluster: copy the appropriate part into the buffer */
    if(remaining)
        return fat_read_cluster_partial(vfs, cluster, buffer_, 0, remaining);

    return count;
}


/*
    fat_write() - write <count> blocks from <buffer> into the file indicated by <node>.  Fail if the
    write would run past the end of the file.
*/
s32 fat_write(vfs_t * const vfs, fs_node_t * const node, const void * const buffer, u32 offset,
              ks32 count)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u8 *buffer_;
    u32 cluster, remaining;
    s32 ret;

    if(count < 0)
        return -EINVAL;

    /* Ensure that the file is long enough for the write */
    if(((node->size + (fs->bytes_per_cluster - 1)) / fs->bytes_per_cluster) <
       ((offset + count + fs->sectors_per_cluster - 1) / fs->sectors_per_cluster))
        return -EINVAL;

    buffer_ = (u8 *) buffer;
    cluster = node->first_block;
    remaining = count;

    /* Find the first cluster containing the blocks to be written */
    for(cluster = node->first_block; offset >= fs->sectors_per_cluster;
        offset -= fs->sectors_per_cluster)
    {
        ret = fat_get_next_cluster(vfs, cluster, &cluster);
        if(ret != SUCCESS)
            return ret;

        if(FAT_CHAIN_END(cluster))
            return -EINVAL;     /* Premature end-of-chain: this indicates corruption. */
    }

    /* Write the appropriate part of this cluster */
    ret = fat_write_cluster_partial(vfs, cluster, buffer_, offset,
                                    fs->sectors_per_cluster - offset);
    if(ret != SUCCESS)
        return ret;

    remaining -= fs->sectors_per_cluster - offset;
    buffer_ += BLOCK_SIZE * (fs->sectors_per_cluster - offset);

    /* Copy additional clusters, as needed */
    for(; remaining >= fs->sectors_per_cluster;
        remaining -= fs->sectors_per_cluster, buffer_ += BLOCK_SIZE * fs->sectors_per_cluster)
    {
        ret = fat_get_next_cluster(vfs, cluster, &cluster);
        if(ret != SUCCESS)
            return ret;

        if(FAT_CHAIN_END(cluster))
            return -EINVAL;     /* Premature end-of-chain: this indicates corruption. */

        ret = fat_write_cluster(vfs, cluster, buffer_);
        if(ret != SUCCESS)
            return ret;
    }

    /* Write the final blocks into the last cluster */
    if(remaining)
    {
        ret = fat_get_next_cluster(vfs, cluster, &cluster);
        if(ret != SUCCESS)
            return ret;

        if(FAT_CHAIN_END(cluster))
            return -EINVAL;     /* Premature end-of-chain: this indicates corruption. */

        ret = fat_write_cluster_partial(vfs, cluster, buffer_, 0, remaining);
        if(ret != SUCCESS)
            return ret;
    }

    return count;
}


/*
    fat_create_node() - create a new directory entry

    FIXME: we mustn't allow this fn to extend the root directory, for some reason
*/
s32 fat_create_node(vfs_t *vfs, u32 parent_block, fs_node_t *node)
{
    fat_dir_ctx_t *dir_ctx;
    s8 lfn[FAT_LFN_MAX_LEN + 1];
    s32 lfn_len, ret;

    /* Open parent node */      /* TODO - remove ref to parent_block */
    if((ret = fat_open_dir(vfs, parent_block, (void **) &dir_ctx)) != SUCCESS)
        return ret;

    /*
        Scan the directory, looking for either an entry with a matching name (and fail if one is
        found) or the end of the directory entries.
    */
    while(!FAT_CHAIN_END(dir_ctx->block))
    {
        /* Read the cluster into dir_ctx->buffer (which was alloc'ed by fat_open_dir() */
        ret = fat_read_cluster(vfs, dir_ctx->block, dir_ctx->buffer);
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
                    (sizeof(fat_lfn_node_t) *
                        ((strlen(node->name) + (FAT_LFN_PART_TOTAL_LEN - 1)) /
                            FAT_LFN_PART_TOTAL_LEN)) +
                    sizeof(fat_node_t);

                /* Is there enough space in this cluster? */
                if(bytes_needed > (u32) (dir_ctx->buffer_end - dir_ctx->de))
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
                        append short-filename entry, including other node data
                        type(new_node)==dir?
                            create new empty dir structure (allocate one cluster, zero it)
                            set size=0
                            create "." and ".." entries
                            ...more here
                        type(new_node)==file?
                            set size=0
                */
            }
            else if((u8) dir_ctx->de->file_name[0] == FAT_DIRENT_UNUSED)
                continue;       /* This entry represents a deleted item */
            else if(dir_ctx->de->attribs == FAT_FILEATTRIB_LFN)
            {
                /* This is a long filename component */
                const fat_lfn_node_t * const lde = (const fat_lfn_node_t *) dir_ctx->de;
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
                    its name matches node->name (i.e. the name requested for the new node), fail.
                */
                if(!strcasecmp(node->name, lfn))
                {
                    fat_close_dir(vfs, dir_ctx);
                    return -EEXIST;
                }
            }
        }

        /* Reached the end of the block without finding a complete entry. */
        /* Find the next block in the chain */
        ret = fat_get_next_cluster(vfs, dir_ctx->block, &dir_ctx->block);
        if(ret != SUCCESS)
        {
            fat_close_dir(vfs, dir_ctx);
            return ret;
        }
    }

    fat_close_dir(vfs, dir_ctx);

    return SUCCESS;
}


/*
    fat_reallocate() - reallocate space associated with a file, by truncating or extending its
    cluster chain.  If the file is truncated, any data in detached clusters will become
    inaccessible.  If it is extended, the additional clusters will be zeroed.  If the new length of
    the file, as specified by <new_len>, does not require any clusters to be added to or removed
    from the chain, this function becomes a no-op.
*/
s32 fat_reallocate(vfs_t * const vfs, fs_node_t * const node, ks32 new_len)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    u32 current_len_clusters, new_len_clusters;
    s32 ret;

    current_len_clusters = (node->size + fs->bytes_per_cluster - 1) / fs->bytes_per_cluster;
    new_len_clusters = (new_len + fs->bytes_per_cluster - 1) / fs->bytes_per_cluster;

    if(new_len_clusters > current_len_clusters)
    {
        u32 cluster, next_cluster;

        /* Iterate to the end of the chain */
        for(cluster = node->first_block, next_cluster = 0; !FAT_CHAIN_END(next_cluster);
            cluster = next_cluster)
        {
            /* Extend the cluster chain */
            ret = fat_get_next_cluster(vfs, cluster, &next_cluster);
            if(ret != SUCCESS)
                return ret;
        }

        /* Reached the end of the chain.  Append new clusters. */

    }
    else if(new_len_clusters < current_len_clusters)
    {
        /* Truncate the cluster chain */
    }

    return SUCCESS;
}


s32 fat_stat(vfs_t *vfs, fs_stat_t *st)
{
    UNUSED(vfs);
    UNUSED(st);

    /* TODO */

    return 0;
}


/*
    fat_alloc_cluster() - find a free cluster in the FAT and mark it as allocated by setting it to
    the end-of-chain marker.  Return the cluster ID on success, or -ENOSPC if no free clusters are
    available.  Other errors may be returned by the calls to block_read() and block_write().
*/
s32 fat_alloc_cluster(vfs_t *vfs)
{
    const fat_fs_t * const fs = (const fat_fs_t *) vfs->data;
    fat16_cluster_id sector[BLOCK_SIZE / sizeof(fat16_cluster_id)], start_block,  block;

    /* Calculate starting point, based on the location of the last free cluster found */
    start_block = fs->last_free_cluster / (BLOCK_SIZE / sizeof(fat16_cluster_id));

    block = start_block;
    do
    {
        fat16_cluster_id offset;
        s32 ret;

        ret = block_read(vfs->dev, fs->first_fat_sector + block, &sector);
        if(ret != SUCCESS)
            return ret;

        for(offset = 0; offset < BLOCK_SIZE / sizeof(fat16_cluster_id); ++offset)
        {
            if(!sector[offset])
            {
                /* Found a free cluster */
                sector[offset] = N2LE16(FAT_CHAIN_TERMINATOR);

                ret = block_write(vfs->dev, fs->first_fat_sector + block, &sector);
                if(ret != SUCCESS)
                    return ret;

                return (block * (BLOCK_SIZE / sizeof(fat16_cluster_id))) + offset;
            }
        }

        if(++block == fs->sectors_per_fat)
            block = 0;
    } while(block != start_block);

    return -ENOSPC;     /* No free nodes found */
}



/*
    fat_generate_basis_name() - generate a "basis name" from a long filename.  E.g.:

        "Letter to the bank manager.doc" (long fn) --> "LETTER~1.DOC" (basis name)

    "tailnum" specifies the numeric tail number (the "~1" part).  No numeric tail is generated if
    the long filename can be fit losslessly into basis_name.  "tailnum" must be >=1.  "basis_name"
    must point to a buffer of at least 11 bytes.

    Note: this code does not properly support Unicode.
*/
s32 fat_generate_basis_name(s8 * lfn, u32 tailnum, char * const basis_name)
{
    /*
        This function implements the basis-name generation algorithm specified in the Microsoft
        document "FAT General Specification".
    */
    s8 *buf, *p;
    u16 u, taillen;

    /*
        1. Strip all leading and embedded spaces from the long name
        2. Strip all leading periods ('.') from the long name
        3. The UNICODE name passed to the file system is converted to upper case.
        4. This step is skipped, because this code does not yet support Unicode:
            The upper cased UNICODE name is converted to OEM.
            if(the uppercased UNICODE glyph does not exist as an OEM glyph in the OEM code page)
                or(the OEM glyph is invalid in an 8.3 name)
            {
                Replace the glyph to an OEM '_' (underscore) character.
                Set a "lossy conversion" flag.
            }
    */

    for(; isspace(*lfn); ++lfn)     /* Step over all leading whitespace     */
        ;

    for(; *lfn == '.'; ++lfn)       /* Step over all leading periods ('.')  */
        ;

    buf = CHECKED_KMALLOC(strlen(lfn) + 1);

    /* Copy lfn to buf, upper-casing it and ignoring any embedded whitespace */
    for(p = buf; *lfn; ++lfn)
        if(!isspace(*lfn))
            *p++ = toupper(*lfn);

    /*
        5.  While(not at end of the long name) and(char is not a period) and(chars copied < 8)
            {
                Copy characters into primary portion of the basis name
            }
    */
    for(u = 0; *buf && (*buf != '.') && (u < FAT_FILENAME_LEN); ++u)
        basis_name[u] = *buf++;

    kfree(buf);     /* Finished with the processed version of the LFN */

    while(u < FAT_FILENAME_LEN)
        basis_name[u] = ' ';

    /*
        6.  Insert a dot at the end of the primary components of the basis-name iff the basis name
            has an extension after the last period in the name.
        7.  Scan for the last embedded period in the long name.
                If(the last embedded period was found)
                {
                    While(not at end of the long name) and (chars copied < 3)
                    {
                        Copy characters into extension portion of the basis name
                    }
                }
    */

    p = strrchr(lfn, '.');
    if(p)
    {
        for(u = 0; *p && (u < FAT_FILEEXT_LEN); ++u)
            basis_name[u + FAT_FILENAME_LEN] = *p++;
    }

    /*
        The Numeric-Tail Generation Algorithm

        If(a "lossy conversion" was not flagged)
            and(the long name fits within the 8.3 naming conventions)
            and(the basis-name does not collide with any existing short name
        {
            The short name is only the basis-name without the numeric tail.
        }
        else
        {
            Insert a numeric-tail "~n" to the end of the primary name such that the value of the
            "~n" is chosen so that the name thus formed does not collide with any existing short
            name and that the primary name does not exceed eight characters in length.
        }

        Note: this function does things a bit differently.  There is no concept of a "lossy"
        conversion, as Unicode is not yet supported.  Also, if tailnum != 0, we always add the
        requested tailnum.
    */

    if(tailnum)
    {
        s8 *tailptr;

        taillen = tailnum ? log10(tailnum) + 2 : 0;     /* Number of chars required for tailnum */

        tailptr = basis_name + FAT_FILENAME_LEN - taillen;
        *tailptr++ = '~';

        sprintf(tailptr, "%u", tailnum);
    }

    return SUCCESS;
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
#ifdef FAT_DEBUG
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
#endif /* 0 */

#endif /* WITH_FS_FAT */
