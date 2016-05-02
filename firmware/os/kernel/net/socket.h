#ifndef KERNEL_NET_SOCKET_H_INC
#define KERNEL_NET_SOCKET_H_INC
/*
    Socket implementation

    Part of ayumos


    (c) Stuart Wallace, April 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/net.h>			// FIXME needed?

s32 socket_init();
s32 socket_create(s32 domain, s32 type, s32 protocol);

#endif
