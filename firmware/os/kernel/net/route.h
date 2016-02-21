#ifndef KERNEL_NET_ROUTE_H_INC
#define KERNEL_NET_ROUTE_H_INC
/*
    Network packet routing functions

    Part of ayumos


    (c) Stuart Wallace, February 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/net.h>


s32 route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr, net_address_t *hw_addr);

#endif
