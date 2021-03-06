#ifndef KERNEL_INCLUDE_NET_PACKET_H_INC
#define KERNEL_INCLUDE_NET_PACKET_H_INC
/*
    Network packet abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/types.h>
#include <kernel/include/net/address.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/protocol.h>


/* Object representing a network packet */
typedef struct net_packet net_packet_t;


s32 net_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t * const iface,
                     net_packet_t **packet);
s32 net_packet_clone(const net_packet_t * const packet, net_packet_t ** new_packet);
void net_packet_free(net_packet_t *packet);
void net_packet_reset(net_packet_t *packet);
void *net_packet_get_start(net_packet_t * const packet);
net_iface_t *net_packet_get_interface(net_packet_t * const packet);
void net_packet_set_interface(net_packet_t * const packet, net_iface_t * const iface);
u32 net_packet_get_len(const net_packet_t * const packet);
u32 net_packet_get_buffer_len(const net_packet_t * const packet);
s32 net_packet_set_len(net_packet_t * const packet, ku32 new_len);
net_protocol_t net_packet_get_proto(net_packet_t * const packet);
void net_packet_set_proto(net_packet_t * const packet, const net_protocol_t proto);
s32 net_packet_insert(net_packet_t * const packet, ku32 len);
s32 net_packet_consume(net_packet_t * const packet, ku32 len);
s32 net_packet_encapsulate(net_packet_t * const packet, const net_protocol_t proto, ku32 len);

#endif /* WITH_NETWORKING */
#endif
