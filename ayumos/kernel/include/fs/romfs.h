#ifndef KERNEL_INCLUDE_FS_ROMFS_H_INC
#define KERNEL_INCLUDE_FS_ROMFS_H_INC
/*
    romfs (trivial read-only file system) support

    Part of ayumos


    (c) Stuart Wallace, January 2016
*/

#ifdef WITH_FS_ROMFS

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/fs/vfs.h>


/* Meaningless "magic number" which validates a romfs superblock */
#define ROMFS_SUPERBLOCK_MAGIC      (0x6c1f39e4)

/* Flags associated with a romfs_node_t */
#define RN_FILE         (0x0001)    /* Node represents a file       */
#define RN_DIR          (0x0002)    /* Node represents a directory  */


typedef struct romfs_superblock
{
    u32     magic;              /* = ROMFS_SUPERBLOCK_MAGIC                         */
    u32     len;                /* Length of the entire romfs, including superblock */
    time_t  cdate;              /* Timestamp at which the fs image was created      */
    u16     nnodes;             /* Number of romfs_node_t structures in the fs      */
    char    label[16];          /* Zero-terminated volume label (name) string       */
} romfs_superblock_t;


/* romfs_node_t: metadata for a "node", i.e. a directory entry, within the file system */
typedef struct romfs_node
{
    u16         id;             /* Unique identifier of this node                               */
    u16         parent_id;      /* Identifier of this node's parent                             */
    u32         size;           /* Size of this node's data block; zero for directory nodes     */
    u32         offset;         /* Offset of the data block, from the start of the superblock   */
    u32         name_offset;    /* Offset of the node name, from the start of the superblock    */
    time_t      mdate;          /* Node last-modification timestamp                             */
    u16         uid;            /* Owner user ID                                                */
    u16         gid;            /* Owner group ID                                               */
    file_perm_t permissions;    /* Node permissions                                             */
    u16         flags;          /* Flags (see constants defined above)                          */
} romfs_node_t;

vfs_driver_t g_romfs_ops;

#endif /* WITH_FS_ROMFS */
#endif
