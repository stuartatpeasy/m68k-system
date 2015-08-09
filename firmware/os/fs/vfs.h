#ifndef FS_VFS_H_INC
#define FS_VFS_H_INC
/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <stdio.h>
#include "device/device.h"
#include "include/types.h"
#include "include/defs.h"

#define MAX_FILESYSTEMS         (16)


typedef struct fs_stat
{
    u32 total_blocks;
    u32 free_blocks;
    ks8 *label;
} fs_stat_t;

typedef enum fsnode_type
{
    FSNODE_TYPE_DIR,
    FSNODE_TYPE_FILE
} fsnode_type_t;

typedef struct fsnode   /* like a dirent?  call it dirent instead? */
{
    enum fsnode_type type;      /* dir, file, etc. */
    const char *name;
    u16 uid;
    u16 gid;
    u16 permissions;            /* e.g. rwxsrwxsrwxt */
    u16 flags;                  /* read-only, hidden, system, ... */
    u32 size;
    u32 ctime;
    u32 mtime;
    u32 atime;
    u32 first_node;
    /* how to link to clusters? */
} fsnode_t;

struct vfs;
typedef struct vfs vfs_t;

typedef struct vfs_driver
{
    ks8 *name;
    s32 (*init)();
    s32 (*mount)(vfs_t *vfs);
    s32 (*umount)(vfs_t *vfs);
    s32 (*fsnode_get)(vfs_t *vfs, u32 node, fsnode_t * const fsn);
    s32 (*fsnode_put)(vfs_t *vfs, u32 node, const fsnode_t * const fsn);
    u32 (*locate)(vfs_t *vfs, u32 node, const char * const path);
    s32 (*stat)(vfs_t *vfs, fs_stat_t *st);
} vfs_driver_t;


struct vfs
{
    vfs_driver_t *driver;
    device_t *dev;
    void *superblock;
    void *data;         /* fs-specific stuff */
};


s32 vfs_init();
vfs_driver_t *vfs_get_driver_by_name(ks8 * const name);


/*
    mounts: [{"mountpoint" -> vfs_t}, ...]

    what is an fsnode?
    - must be some sort of standardised struct
    - not file data
    - represents a directory or a file (ie. filesystem tree node)
    - enables enumeration of directory contents


    gives:

        vfs_t vfs;


        vfs.ops->mount(&vfs);

        // Look up node for <mountpoint>/home/swallace
        u32 home_node = vfs.ops->locate(&vfs, 0, "home");
        u32 my_node = vfs.ops->locate(&vfs, home_node, "swallace");

        vfs.ops->fsnode_get(&vfs, my_node);



    would be nice:

    vfs->ops->mount()
*/

#endif

