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

typedef struct vfs_dirent
{
    enum fsnode_type type;      /* dir, file, etc. */
    s8 name[NAME_MAX_LEN + 1];
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
} vfs_dirent_t;

struct vfs;
typedef struct vfs vfs_t;

typedef struct vfs_driver
{
    ks8 *name;
    s32 (*init)();
    s32 (*mount)(vfs_t *vfs);
    s32 (*umount)(vfs_t *vfs);
    s32 (*open_dir)(vfs_t *vfs, u32 node, void **ctx);
    s32 (*read_dir)(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent);
    s32 (*close_dir)(vfs_t *vfs, void *ctx);
    s32 (*stat)(vfs_t *vfs, fs_stat_t *st);
} vfs_driver_t;


struct vfs
{
    vfs_driver_t *driver;
    device_t *dev;
    void *superblock;
    void *data;         /* fs-specific stuff */
};

/* Permissions bits */
#define VFS_PERM_UR         (0x0800)        /* User (owner) read        */
#define VFS_PERM_UW         (0x0400)        /* User (owner) write       */
#define VFS_PERM_UX         (0x0200)        /* User (owner) execute     */
#define VFS_PERM_UT         (0x0100)        /* User (owner) sticky      */
#define VFS_PERM_GR         (0x0080)        /* Group read               */
#define VFS_PERM_GW         (0x0040)        /* Group write              */
#define VFS_PERM_GX         (0x0020)        /* Group execute            */
#define VFS_PERM_GT         (0x0010)        /* Group sticky             */
#define VFS_PERM_OR         (0x0008)        /* Other (world) read       */
#define VFS_PERM_OW         (0x0004)        /* Other (world) write      */
#define VFS_PERM_OX         (0x0002)        /* Other (world) execute    */
#define VFS_PERM_OT         (0x0001)        /* Other (world) sticky     */

/* Common combinations of perms */
#define VFS_PERM_URWX       (VFS_PERM_UR | VFS_PERM_UW | VFS_PERM_UX)
#define VFS_PERM_GRWX       (VFS_PERM_GR | VFS_PERM_GW | VFS_PERM_GX)
#define VFS_PERM_ORWX       (VFS_PERM_OR | VFS_PERM_OW | VFS_PERM_OX)

#define VFS_PERM_UGORWX     (VFS_PERM_URWX | VFS_PERM_GRWX | VFS_PERM_ORWX)


s32 vfs_init();
vfs_driver_t *vfs_get_driver_by_name(ks8 * const name);

#endif

