/*
    Path-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/fs/path.h>


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
    return path[0] == DIR_SEPARATOR ? 1 : 0;
}
