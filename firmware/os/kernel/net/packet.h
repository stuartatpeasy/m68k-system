#ifndef KERNEL_NET_PACKET_H_INC
#define KERNEL_NET_PACKET_H_INC
/*
    Network packet abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/net/net.h>
#include <kernel/net/interface.h>
#include <kernel/net/protocol.h>


/* Object representing a network packet */
typedef struct net_packet net_packet_t;
struct net_packet
{
    net_iface_t *           iface;
    net_protocol_t          proto;
    void *                  start;
    u32                     len;        /* Actual amount of data in the buffer  */
    buffer_t                raw;
};


s32 net_packet_alloc(const net_protocol_t proto, const net_address_t * const addr, ku32 len,
                     const net_iface_t * const iface, net_packet_t **packet);
void net_packet_free(net_packet_t *packet);
void *net_packet_get_start(net_packet_t * const packet);
u32 net_packet_get_len(net_packet_t * const packet);
u32 net_packet_get_len(net_packet_t * const packet);
net_protocol_t net_packet_get_proto(net_packet_t * const packet);
void net_packet_set_proto(const net_protocol_t proto, net_packet_t * const packet);
s32 net_packet_insert(ku32 len, net_packet_t * const packet);
s32 net_packet_consume(ku32 len, net_packet_t * const packet);
s32 net_packet_encapsulate(const net_protocol_t proto, ku32 len, net_packet_t * const packet);


#endif
