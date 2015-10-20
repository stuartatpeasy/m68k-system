#ifndef KERNEL_USER_H_INC
#define KERNEL_USER_H_INC
/*
    Declarations and functions relating to users and groups

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <include/defs.h>
#include <include/types.h>


typedef u16 uid_t;
typedef u16 gid_t;

#define ROOT_USER       ((uid_t) 0)
#define ROOT_GROUP      ((gid_t) 0)


#endif
