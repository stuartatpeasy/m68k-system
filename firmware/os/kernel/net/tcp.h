#ifndef KERNEL_NET_TCP_H_INC
#define KERNEL_NET_TCP_H_INC
/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/net.h>


s32 tcp_handle_packet(net_iface_t *iface, net_packet_t *packet, net_packet_t **response);

#endif
