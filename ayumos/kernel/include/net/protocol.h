#ifndef KERNEL_INCLUDE_NET_PROTOCOL_H_INC
#define KERNEL_INCLUDE_NET_PROTOCOL_H_INC
/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/types.h>
#include <kernel/include/net/net.h>
#include <kernel/include/net/address.h>
#include <kernel/include/net/interface.h>


/* Functions implemented by a network protocol driver */
typedef struct net_proto_fns
{
    s32     (*rx)(net_address_t *src, net_address_t *dest, net_packet_t *packet);
    s32     (*tx)(net_address_t *src, net_address_t *dest, net_packet_t *packet);
    s32     (*addr_compare)(const net_address_t * const a1, const net_address_t * const a2);
    s32     (*packet_alloc)(const net_address_t * const addr, ku32 len,
                            net_iface_t * const iface, net_packet_t **packet);
    s32     (*route_get_iface)(const net_address_t * const addr, net_iface_t **iface);
} net_proto_fns_t;

/* Network protocol driver object */
typedef struct net_proto_driver net_proto_driver_t;


s32 net_protocol_register_driver(const net_protocol_t proto, const char * const name,
                                 net_proto_fns_t * const f);
void net_proto_fns_struct_init(net_proto_fns_t * const f);
net_proto_driver_t *net_protocol_get_first();
net_proto_driver_t *net_protocol_get_next(const net_proto_driver_t * const proto);
const char *net_protocol_get_name(const net_proto_driver_t * const proto);
s32 net_protocol_rx(net_address_t *src, net_address_t *dest, net_packet_t * const packet);
s32 net_protocol_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 net_protocol_addr_compare(const net_protocol_t proto, const net_address_t * const a1,
                              const net_address_t * const a2);
s32 net_protocol_packet_alloc(const net_protocol_t proto, const net_address_t * const addr,
                              ku32 len, net_iface_t * const iface, net_packet_t **packet);
s32 net_route_get_iface(const net_address_t * const addr, net_iface_t **iface);
net_protocol_t net_protocol_from_address(const net_address_t * const addr);
net_protocol_t net_protocol_hwproto_from_address(const net_address_t * const addr);

#endif /* WITH_NETWORKING */
#endif
