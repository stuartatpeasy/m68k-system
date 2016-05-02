#ifndef KERNEL_NET_IPV4ROUTE_H_INC
#define KERNEL_NET_IPV4ROUTE_H_INC
/*
    IPv4 network packet routing functions

    Part of ayumos


    (c) Stuart Wallace, February 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>


/* An IPv4 route */
typedef struct ipv4_route
{
    net_iface_t     *iface;
    ipv4_addr_t     dest;
    ipv4_addr_t     mask;
    ipv4_addr_t     gateway;
    u16             metric;
    u16             flags;
} ipv4_route_t;

/* IPv4 routing table entry */
typedef struct ipv4_rt_item ipv4_rt_item_t;
struct ipv4_rt_item
{
    ipv4_route_t    r;
    ipv4_rt_item_t  *next;
};

/* The IPv4 routing table */
ipv4_rt_item_t *g_ipv4_routes;


/* Routing table flags */
#define IPV4_ROUTE_UP           BIT(0)  /* Route is up (active)             */
#define IPV4_ROUTE_HOST         BIT(1)  /* Target is a host                 */
#define IPV4_ROUTE_GATEWAY      BIT(2)  /* Use a gateway                    */
#define IPV4_ROUTE_REJECT       BIT(3)  /* A "reject" route - drop traffic  */


s32 ipv4_route_add(const ipv4_route_t * const r);

s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr);

#endif
