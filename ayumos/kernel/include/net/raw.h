#ifndef KERNEL_INCLUDE_NET_RAW_H_INC
#define KERNEL_INCLUDE_NET_RAW_H_INC
/*
    "Raw" protocol implementation

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/include/net/net.h>


s32 raw_init();
s32 raw_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t * const iface,
                     net_packet_t **packet);

#endif
