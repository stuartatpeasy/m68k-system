/*
    Functions relating to users and groups

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/user.h>
#include <klibc/include/errno.h>


s32 group_member(const uid_t uid, const gid_t gid)
{
    UNUSED(uid);
    UNUSED(gid);
    /* TODO */

    return ENOENT;
}
