#ifndef KERNEL_NET_ROUTE_H_INC
#define KERNEL_NET_ROUTE_H_INC
/*
    Network routing abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/net/net.h>
#include <kernel/net/interface.h>

typedef struct net_route net_route_t;


net_iface_t *net_route_get(const net_address_t *addr);

#endif
