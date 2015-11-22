#ifndef KERNEL_NET_ICMP_H_INC
#define KERNEL_NET_ICMP_H_INC
/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/net.h>


s32 icmp_handle_packet(net_iface_t *iface, const void *packet, u32 len);

#endif
