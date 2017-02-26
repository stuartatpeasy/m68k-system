#ifndef KERNEL_INCLUDE_FS_VFS_H_INC
#define KERNEL_INCLUDE_FS_VFS_H_INC
/*
    Virtual file system abstraction

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <kernel/include/device/device.h>
#include <kernel/include/fs/node.h>
#include <kernel/include/types.h>
#include <kernel/include/defs.h>
#include <klibc/include/stdio.h>


typedef struct vfs vfs_t;

typedef struct fs_stat
{
    u32 total_blocks;
    u32 free_blocks;
    ks8 *label;
} fs_stat_t;

typedef struct vfs_driver
{
    ks8 *name;
    s32 (*init)();
    s32 (*mount)(vfs_t *vfs);
    s32 (*unmount)(vfs_t *vfs);
    s32 (*get_root_node)(vfs_t *vfs, fs_node_t **node);
    s32 (*open_dir)(vfs_t *vfs, u32 block, void **ctx);
    s32 (*read_dir)(vfs_t *vfs, void *ctx, ks8 * const name, fs_node_t *node);
    s32 (*close_dir)(vfs_t *vfs, void *ctx);
    s32 (*read)(vfs_t * const vfs, fs_node_t * const node, void * const buffer, u32 offset,
                ks32 count);
    s32 (*write)(vfs_t * const vfs, fs_node_t * const node, const void * const buffer, u32 offset,
                 ks32 count);
    s32 (*reallocate)(vfs_t * const vfs, fs_node_t * const node, ks32 new_len);
    s32 (*stat)(vfs_t *vfs, fs_stat_t *st);
} vfs_driver_t;

typedef struct vfs_dir_ctx
{
    vfs_t *vfs;
    void *ctx;      /* FIXME rename this to "data" */
} vfs_dir_ctx_t;


struct vfs
{
    vfs_driver_t *driver;
    dev_t *dev;
    u32 root_block;
    void *data;         /* fs-specific stuff */
};


s32 vfs_init();
s32 vfs_attach(vfs_driver_t * const driver, dev_t * const dev, vfs_t **vfs);
s32 vfs_detach(vfs_t *vfs);
vfs_driver_t *vfs_get_driver_by_name(ks8 * const name);
s32 vfs_unmount(vfs_t *vfs);
s32 vfs_get_root_node(vfs_t *vfs, fs_node_t **node);
s32 vfs_open_dir(vfs_t * const vfs, fs_node_t * const node, vfs_dir_ctx_t **ctx);
s32 vfs_read_dir(vfs_dir_ctx_t *ctx, ks8 * const name, fs_node_t *node);
s32 vfs_close_dir(vfs_dir_ctx_t *ctx);
s32 vfs_read(vfs_t * const vfs, fs_node_t * const node, void * const buffer, ku32 offset,
             ks32 count);
s32 vfs_write(vfs_t * const vfs, fs_node_t * const node, const void * const buffer, ku32 offset,
              s32 count);
s32 vfs_get_child_node(fs_node_t *parent, const char * const child, vfs_t **vfs, fs_node_t **node);

#endif
