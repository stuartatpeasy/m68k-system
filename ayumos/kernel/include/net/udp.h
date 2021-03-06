#ifndef KERNEL_INCLUDE_NET_UDP_H_INC
#define KERNEL_INCLUDE_NET_UDP_H_INC
/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING
#ifdef WITH_NET_UDP

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/net.h>


/* UDP header */
typedef struct udp_hdr
{
    u16             src_port;
    u16             dest_port;
    u16             len;
    u16             cksum;
} udp_hdr_t;


s32 udp_init();
s32 udp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 udp_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 udp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet);

#endif /* WITH_NET_UDP */
#endif /* WITH_NETWORKING */
#endif
