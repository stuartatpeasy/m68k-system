#ifndef KERNEL_INCLUDE_NET_SOCKET_H_INC
#define KERNEL_INCLUDE_NET_SOCKET_H_INC
/*
    Socket implementation

    Part of ayumos


    (c) Stuart Wallace, April 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/net.h>


s32 socket_init();
s32 socket_create(s32 domain, s32 type, s32 protocol);

#endif /* WITH_NETWORKING */
#endif
