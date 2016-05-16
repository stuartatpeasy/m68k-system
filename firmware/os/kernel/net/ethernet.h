#ifndef KERNEL_NET_ETHERNET_H_INC
#define KERNEL_NET_ETHERNET_H_INC
/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/interface.h>
#include <kernel/net/ipv4.h>        // FIXME - why is this here?
#include <kernel/net/net.h>
#include <kernel/util/buffer.h>


#define MAC_ADDR_EQUAL(x, y)    \
    (x->w[0] == y->w[0]) && (x->w[1] == y->w[1]) && (x->w[2] == y->w[2])

/* Six-byte MAC address. NOTE: bytes stored in network order. */
typedef union mac_addr
{
    u8 b[6];
    u16 w[3];
} mac_addr_t;

const net_address_t g_eth_broadcast;


/* Enumeration of Ethertype values */
typedef enum ethertype
{
    ethertype_ipv4  = 0x0800,
    ethertype_arp   = 0x0806
} ethertype_t;


/* Ethernet II frame header */
typedef struct eth_hdr
{
    mac_addr_t      dest;
    mac_addr_t      src;
    u16             type;   /* u16, not (enum) ethertype_t, because size is fixed */
} eth_hdr_t;

typedef u32 eth_cksum_t;    /* Ethernet checksum (the last four bytes of an Ethernet frame) */


s32 eth_init();
s32 eth_rx(net_packet_t *packet);
s32 eth_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
//s32 eth_reply(net_packet_t *packet);      // FIXME
s32 eth_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet);
void eth_make_addr(mac_addr_t *mac, net_address_t *addr);
const mac_addr_t *eth_get_addr(const net_address_t * const addr);
s32 eth_addr_compare(const net_address_t * const a1, const net_address_t * const a2);
net_protocol_t eth_proto_from_ethertype(ku16 ethertype);
s32 eth_print_addr(const net_address_t *addr, char *buf, s32 len);

#endif
