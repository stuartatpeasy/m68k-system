#ifndef KERNEL_USER_H_INC
#define KERNEL_USER_H_INC
/*
    Declarations and functions relating to users and groups

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


typedef u16 uid_t;
typedef u16 gid_t;

#define ROOT_UID        ((uid_t) 0)
#define ROOT_GID        ((gid_t) 0)


s32 group_member(const uid_t uid, const gid_t gid);

#endif
