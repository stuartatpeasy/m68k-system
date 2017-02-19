#ifndef KERNEL_INCLUDE_FS_FILE_H_INC
#define KERNEL_INCLUDE_FS_FILE_H_INC
/*
    File-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/fs/node.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/types.h>
#include <kernel/include/user.h>


/* Flags for file_open() */
#define O_RD            (0x01)      /* Open file for reading                                */
#define O_WR            (0x02)      /* Open file for writing                                */
#define O_CREATE        (0x04)      /* Create file if it does not exist                     */
#define O_APPEND        (0x08)      /* Set offset to EOF                                    */
#define O_EXCL          (0x10)      /* Ensure that the call to file_open() creates the file */

#define O_RDWR (O_RD | O_WR)        /* Open file for reading and writing */


struct file_info
{
    vfs_t *vfs;
    fs_node_t *node;
    u32 offset;
    u16 flags;
    pid_t pid;
};

typedef struct file_info file_info_t;


s32 file_open(ks8 * const path, u16 flags, file_info_t **fp);
s32 file_create(ks8 * const path, file_perm_t perm, fs_node_t **node);

s32 syscall_open(const char * const path, u16 mode);
s32 syscall_create(const char * const path, ku16 mode);
s32 syscall_close(s32 filenum);
s32 syscall_read(const s32 filenum, void * const buf, size_t count);
s32 syscall_write(const s32 filenum, const void * const buf, size_t count);

#endif
