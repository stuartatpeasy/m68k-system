#ifndef FS_VFS_H_INC
#define FS_VFS_H_INC
/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <include/types.h>
#include <include/defs.h>
#include <kernel/device/device.h>
#include <stdio.h>


#define MAX_FILESYSTEMS         (16)

typedef u16 file_perm_t;

struct vfs;
typedef struct vfs vfs_t;

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
    vfs_t           *vfs;
    fsnode_type_t   type;           /* dir, file, etc. */
    s8              name[NAME_MAX_LEN + 1];
    u16             uid;
    u16             gid;
    file_perm_t     permissions;    /* e.g. rwxsrwxsrwx */
    u16             flags;          /* read-only, hidden, system, ... */
    u32             size;
    u32             ctime;
    u32             mtime;
    u32             atime;
    u32             first_node;
    /* how to link to clusters? */
} vfs_dirent_t;

typedef struct vfs_driver
{
    ks8 *name;
    s32 (*init)();
    s32 (*mount)(vfs_t *vfs);
    s32 (*umount)(vfs_t *vfs);
    s32 (*get_root_dirent)(vfs_t *vfs, vfs_dirent_t *dirent);
    s32 (*open_dir)(vfs_t *vfs, u32 node, void **ctx);
    s32 (*read_dir)(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name);
    s32 (*close_dir)(vfs_t *vfs, void *ctx);
    s32 (*stat)(vfs_t *vfs, fs_stat_t *st);
} vfs_driver_t;

typedef struct vfs_dir_ctx
{
    vfs_t *vfs;
    void *ctx;

} vfs_dir_ctx_t;


struct vfs
{
    vfs_driver_t *driver;
    dev_t *dev;
    u32 root_node;
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

/* Definitions used when testing permissions */
#define VFS_PERM_R          (0x0008)        /* Generic read permission      */
#define VFS_PERM_W          (0x0004)        /* Generic write permission     */
#define VFS_PERM_X          (0x0002)        /* Generic execute permission   */

#define VFS_PERM_MASK       (VFS_PERM_R | VFS_PERM_W | VFS_PERM_X)
#define VFS_PERM_SHIFT_U    (8)
#define VFS_PERM_SHIFT_G    (4)
#define VFS_PERM_SHIFT_O    (0)

/* Common combinations of perms */
#define VFS_PERM_URWX       (VFS_PERM_UR | VFS_PERM_UW | VFS_PERM_UX)       /* rwx------ */
#define VFS_PERM_GRWX       (VFS_PERM_GR | VFS_PERM_GW | VFS_PERM_GX)       /* ---rwx--- */
#define VFS_PERM_ORWX       (VFS_PERM_OR | VFS_PERM_OW | VFS_PERM_OX)       /* ------rwx */
#define VFS_PERM_URX        (VFS_PERM_UR | VFS_PERM_UX)                     /* r-x------ */
#define VFS_PERM_GRX        (VFS_PERM_GR | VFS_PERM_GX)                     /* ---r-x--- */
#define VFS_PERM_ORX        (VFS_PERM_OR | VFS_PERM_OX)                     /* ------r-x */

#define VFS_PERM_UGORWX     (VFS_PERM_URWX | VFS_PERM_GRWX | VFS_PERM_ORWX) /* rwxrwxrwx */
#define VFS_PERM_UGORX      (VFS_PERM_URX | VFS_PERM_GRX | VFS_PERM_ORX)    /* r-xr-xr-x */

/* Flags */
#define VFS_FLAG_HIDDEN     (0x0001)        /* Not sure this will be respected  */
#define VFS_FLAG_SYSTEM     (0x0002)        /* Not sure this will be respected  */
#define VFS_FLAG_ARCHIVE    (0x0004)        /* I have no idea what this means   */

s32 vfs_init();
vfs_driver_t *vfs_get_driver_by_name(ks8 * const name);
s32 vfs_open_dir(ks8 *path, vfs_dir_ctx_t *ctx);
s32 vfs_read_dir(vfs_dir_ctx_t *ctx, vfs_dirent_t *dirent, ks8 *const name);
s32 vfs_close_dir(vfs_dir_ctx_t *ctx);
s32 vfs_lookup(ks8 *path, vfs_dirent_t *ent);
s8 *vfs_dirent_perm_str(const vfs_dirent_t * const dirent, s8 *str);

/* Default versions of the functions in vfs_driver_t.  These all return ENOSYS. */
s32 vfs_default_mount(vfs_t *vfs);
s32 vfs_default_umount(vfs_t *vfs);
s32 vfs_default_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent);
s32 vfs_default_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 vfs_default_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name);
s32 vfs_default_close_dir(vfs_t *vfs, void *ctx);
s32 vfs_default_stat(vfs_t *vfs, fs_stat_t *st);

#endif

