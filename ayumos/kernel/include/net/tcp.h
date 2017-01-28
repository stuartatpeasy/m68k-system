#ifndef KERNEL_INCLUDE_NET_TCP_H_INC
#define KERNEL_INCLUDE_NET_TCP_H_INC
/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/net.h>


s32 tcp_init();
s32 tcp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);

#endif /* WITH_NETWORKING */
#endif
