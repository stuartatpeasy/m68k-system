/*
    Path-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/fs/path.h>
#include <kernel/include/fs/vfs.h>
#include <klibc/include/string.h>
#include <klibc/include/stdio.h>        // FIXME


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
    path_open() - walk the specified path
*/
s32 path_open(const char *path)
{
    char *path_canon, *p, *component, sep;
    vfs_t *vfs;
    fs_node_t *parent, *child;
    s32 ret;

    if(!path_is_absolute(path))
        return EINVAL;

    path_canon = strdup(path);
    if(path_canon == NULL)
        return ENOMEM;

    p = path_canonicalise(path_canon);
    parent = NULL;

    /* Get the file system root node */
    vfs = NULL;
    ret = vfs_get_child_node(NULL, NULL, &vfs, &parent);
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

        sep = *p;
        *p = '\0';

        /* Look up component */
        ret = vfs_get_child_node(parent, component, &vfs, &child);
        if(ret != SUCCESS)
        {
            fs_node_free(parent);
            kfree(path_canon);
            return ret;
        }

        /* FIXME - check permissions */

        fs_node_free(parent);
        parent = child;
    } while(sep);

    fs_node_free(parent);
    kfree(path_canon);

    return SUCCESS;
}
