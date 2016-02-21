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
s32 udp_send(net_iface_t *iface, const net_address_t *src_addr, const ipv4_port_t src_port,
             const net_address_t *dest_addr, const ipv4_port_t dest_port, buffer_t *payload);
s32 udp_alloc_packet(net_iface_t *iface, ku32 len, net_packet_t **packet);

#endif
