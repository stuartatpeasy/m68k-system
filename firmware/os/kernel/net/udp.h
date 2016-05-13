#ifndef KERNEL_NET_UDP_H_INC
#define KERNEL_NET_UDP_H_INC
/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>


/* UDP header */
typedef struct udp_hdr
{
    u16             src_port;
    u16             dest_port;
    u16             len;
    u16             cksum;
} udp_hdr_t;


s32 udp_rx(net_packet_t *packet);
s32 udp_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 udp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet);
#endif
