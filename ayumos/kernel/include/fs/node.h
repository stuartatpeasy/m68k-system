#ifndef KERNEL_INCLUDE_FS_NODE_H_INC
#define KERNEL_INCLUDE_FS_NODE_H_INC
/*
    File system node and node-cache management

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


typedef u16 file_perm_t;

typedef enum fsnode_type
{
    FSNODE_TYPE_DIR,
    FSNODE_TYPE_FILE
} fsnode_type_t;


typedef struct fs_node
{
    fsnode_type_t   type;           /* dir, file, etc. */
    char           *name;
    u16             uid;
    u16             gid;
    file_perm_t     permissions;    /* e.g. rwxsrwxsrwx */
    u16             flags;          /* read-only, hidden, system, ... */
    u32             size;
    time_t          ctime;
    time_t          mtime;
    time_t          atime;
    u32             first_block;
    /* how to link to clusters? */
} fs_node_t;


/* Permissions bits */
#define FS_PERM_UR          (0x0800)        /* User (owner) read        */
#define FS_PERM_UW          (0x0400)        /* User (owner) write       */
#define FS_PERM_UX          (0x0200)        /* User (owner) execute     */
#define FS_PERM_UT          (0x0100)        /* User (owner) sticky      */
#define FS_PERM_GR          (0x0080)        /* Group read               */
#define FS_PERM_GW          (0x0040)        /* Group write              */
#define FS_PERM_GX          (0x0020)        /* Group execute            */
#define FS_PERM_GT          (0x0010)        /* Group sticky             */
#define FS_PERM_OR          (0x0008)        /* Other (world) read       */
#define FS_PERM_OW          (0x0004)        /* Other (world) write      */
#define FS_PERM_OX          (0x0002)        /* Other (world) execute    */
#define FS_PERM_OT          (0x0001)        /* Other (world) sticky     */

/* Definitions used when testing permissions */
#define FS_PERM_R           (0x0008)        /* Generic read permission      */
#define FS_PERM_W           (0x0004)        /* Generic write permission     */
#define FS_PERM_X           (0x0002)        /* Generic execute permission   */

#define FS_PERM_MASK        (FS_PERM_R | FS_PERM_W | FS_PERM_X)
#define FS_PERM_SHIFT_U     (8)
#define FS_PERM_SHIFT_G     (4)
#define FS_PERM_SHIFT_O     (0)

/* Common combinations of perms */
#define FS_PERM_URWX        (FS_PERM_UR | FS_PERM_UW | FS_PERM_UX)          /* rwx------ */
#define FS_PERM_GRWX        (FS_PERM_GR | FS_PERM_GW | FS_PERM_GX)          /* ---rwx--- */
#define FS_PERM_ORWX        (FS_PERM_OR | FS_PERM_OW | FS_PERM_OX)          /* ------rwx */
#define FS_PERM_URX         (FS_PERM_UR | FS_PERM_UX)                       /* r-x------ */
#define FS_PERM_GRX         (FS_PERM_GR | FS_PERM_GX)                       /* ---r-x--- */
#define FS_PERM_ORX         (FS_PERM_OR | FS_PERM_OX)                       /* ------r-x */

#define FS_PERM_UGORWX      (FS_PERM_URWX | FS_PERM_GRWX | FS_PERM_ORWX)    /* rwxrwxrwx */
#define FS_PERM_UGORX       (FS_PERM_URX | FS_PERM_GRX | FS_PERM_ORX)       /* r-xr-xr-x */

/* Flags */
#define FS_FLAG_HIDDEN      (0x0001)        /* Not sure this will be respected  */
#define FS_FLAG_SYSTEM      (0x0002)        /* Not sure this will be respected  */
#define FS_FLAG_ARCHIVE     (0x0004)        /* I have no idea what this means   */

s8 *fs_node_perm_str(const fs_node_t * const node, s8 *str);
s32 fs_node_alloc(fs_node_t **node);
s32 fs_node_set_name(fs_node_t *node, const char * const name);
void fs_node_free(fs_node_t *node);
s32 fs_node_check_perms(const file_perm_t op, const fs_node_t * const node);

#endif

