/*
    Path-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/fs/path.h>
#include <kernel/include/fs/vfs.h>
#include <klibc/include/string.h>


void path_get_absolute(ks8 *path)
{
    UNUSED(path);
}


/*
    path_is_absolute() - return true if the supplied path is absolute (i.e. begins with a
    DIR_SEPARATOR character), false otherwise.
*/
s32 path_is_absolute(ks8 *path)
{
    return ((path != NULL) && (path[0] == DIR_SEPARATOR)) ? 1 : 0;
}


/*
    path_open() - walk the absolute path specified in <path>.  Fail with EINVAL if <path> is not
    absolute, and ENOMEM if any memory allocation fails.  Fail with EPERM if a permissions error
    occurs while traversing the path.

    TODO - handle symlinks
*/
s32 path_open(const char *path, vfs_t **vfs, fs_node_t **node)
{
    char *path_canon, *p, *component, sep;
    vfs_t *vfs_;
    fs_node_t *parent, *child;
    s32 ret;

    if(!path_is_absolute(path))
        return -EINVAL;

    path_canon = strdup(path);
    if(path_canon == NULL)
        return -ENOMEM;

    p = path_canonicalise(path_canon);
    parent = NULL;

    /* Get the file system root node */
    vfs_ = NULL;
    ret = vfs_get_child_node(NULL, NULL, &vfs_, &parent);
    if(ret != SUCCESS)
    {
        kfree(path_canon);
        return ret;
    }

    do
    {
        /* Extract the next path component */
        for(component = ++p; *p && (*p != DIR_SEPARATOR); ++p)
            ;

        sep = *p;       /* if sep == '\0', this is the last component in the path */
        *p = '\0';

        /* Look up component */
        ret = vfs_get_child_node(parent, component, &vfs_, &child);
        if(ret != SUCCESS)
        {
            fs_node_free(parent);
            kfree(path_canon);
            return ret;
        }

        if(sep)
        {
            ret = fs_node_check_perms(FS_PERM_R | FS_PERM_X, child);
            if(ret != SUCCESS)
            {
                fs_node_free(parent);
                fs_node_free(child);
                kfree(path_canon);
                return ret;
            }
        }

        fs_node_free(parent);
        parent = child;
    } while(sep);

    kfree(path_canon);

    if(vfs != NULL)
        *vfs = vfs_;

    if(node != NULL)
        *node = parent;

    return SUCCESS;
}
