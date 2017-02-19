/*
    File-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/include/fs/file.h>
#include <kernel/include/fs/path.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/process.h>


/*
    file_open() - open a file
*/
s32 file_open(ks8 * const path, u16 flags, file_info_t **fp)
{
    vfs_t *vfs;
    fs_node_t *node;
    file_perm_t perm_needed;
    file_info_t *fp_new;
    s32 ret;

    /* At least one of O_RD and O_WR must be present in flags */
    if(flags & O_RD)
        perm_needed = (flags & O_WR) ? FS_PERM_R | FS_PERM_W : FS_PERM_R;
    else if(flags & O_WR)
        perm_needed = FS_PERM_W;
    else
        return -EINVAL;

    /* Determine whether the path represents an existing fs node */
    ret = path_open(path, &vfs, &node);
    if(ret == SUCCESS)
    {
        /* Node already exists.  Was exclusive creation requested? */
        if(flags & O_EXCL)
        {
            slab_free(node);
            return -EEXIST;
        }

        /* Ensure that the node doesn't represent a directory */
        if(node->type == FSNODE_TYPE_DIR)
        {
            slab_free(node);
            return -EISDIR;
        }

        /* Check perms */
        ret = fs_node_check_perms(perm_needed, node);
        if(ret != SUCCESS)
        {
            slab_free(node);
            return ret;
        }
    }
    else if(ret == -ENOENT)
    {
        /* File does not exist.  Was creation requested? */
        if(flags & O_CREATE)
        {
            ret = file_create(path, proc_current_default_perm(), &node);
            if(ret != SUCCESS)
            {
                slab_free(node);
                return ret;
            }
        }
        else
        {
            slab_free(node);
            return -ENOENT;     /* File does not exist */
        }
    }
    else
    {
        slab_free(node);
        return ret;     /* Something went wrong in vfs_lookup() */
    }

    fp_new = slab_alloc(sizeof(file_info_t));
    if(fp_new == NULL)
    {
        slab_free(node);
        return -ENOMEM;
    }

    fp_new->vfs = vfs;
    fp_new->node = node;
    fp_new->flags = flags;
    fp_new->offset = 0;

    *fp = fp_new;

    return SUCCESS;
}


/*
    file_create() - create a new file at path.
*/
s32 file_create(ks8 * const path, file_perm_t perm, fs_node_t **node)
{
    UNUSED(path);
    UNUSED(perm);
    UNUSED(node);

    return -ENOSYS;
}


/*
    syscall_open() - attempt to open (or create) the file at <path>, taking into account the options
    specified in <mode>.  Return a file descriptor number, or an error code.
*/
s32 syscall_open(const char * const path, u16 mode)
{
    UNUSED(path);
    UNUSED(mode);

    return -ENOSYS;
}


/*
    syscall_create() - attempt to create a file at <path>, taking into account the options specified
    in <mode>.  Return a file descriptor number, or an error code.
*/
s32 syscall_create(const char * const path, u16 mode)
{
    UNUSED(path);
    UNUSED(mode);

    return -ENOSYS;
}


/*
    syscall_close() - close the file descriptor <filenum>.
*/
s32 syscall_close(const s32 filenum)
{
    UNUSED(filenum);

    return -ENOSYS;
}


/*
    syscall_read() - read <count> bytes from the file descriptor <filenum> into <buf>.
*/
s32 syscall_read(const s32 filenum, void * const buf, size_t count)
{
    UNUSED(filenum);
    UNUSED(buf);
    UNUSED(count);

    return -ENOSYS;
}


/*
    syscall_write() - write <count> bytes from <buf> into the file identified by descriptor
    <filenum>.
*/
s32 syscall_write(const s32 filenum, const void * const buf, size_t count)
{
    UNUSED(filenum);
    UNUSED(buf);
    UNUSED(count);

    return -ENOSYS;
}
