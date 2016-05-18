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
typedef struct ipv4_route ipv4_route_t;

/* FIXME: move this struct into ipv4route.c (i.e. make it private) */
struct ipv4_route
{
    net_iface_t     *iface;
    ipv4_addr_t     dest;
    ipv4_addr_t     mask;
    ipv4_addr_t     gateway;
    s16             metric;
    u16             flags;
    s16             prefix_len;
};


/* IPv4 routing table entry */
typedef struct ipv4_rt_item ipv4_rt_item_t;

/* FIXME: move this struct into ipv4route.c (i.e. make it private) */
struct ipv4_rt_item
{
    ipv4_route_t    r;
    ipv4_rt_item_t  *next;
};


/* Routing table flags */
#define IPV4_ROUTE_UP           BIT(0)      /* Route is up (active)             */
#define IPV4_ROUTE_HOST         BIT(1)      /* Target is a host                 */
#define IPV4_ROUTE_GATEWAY      BIT(2)      /* Use a gateway                    */
#define IPV4_ROUTE_REJECT       BIT(3)      /* A "reject" route - drop traffic  */

#define IPV4_ROUTE_METRIC_MIN   (0)         /* Minimum allowable route metric   */
#define IPV4_ROUTE_METRIC_MAX   U16_MAX     /* Maximum allowable route metric   */


s32 ipv4_route_add(const ipv4_route_t * const r);
s32 ipv4_route_delete(const ipv4_route_t * const r);
s32 ipv4_route_get_entry(ipv4_rt_item_t **e);
const ipv4_route_t *ipv4_route_get(const net_address_t * const proto_addr);
net_iface_t *ipv4_route_get_iface(const net_address_t * const proto_addr);
s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr);

#endif
