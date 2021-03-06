#ifndef KERNEL_INCLUDE_NET_ETHERNET_H_INC
#define KERNEL_INCLUDE_NET_ETHERNET_H_INC
/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING
#ifdef WITH_NET_ETHERNET

#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/device/device.h>
#include <kernel/include/types.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/net.h>
#include <kernel/util/buffer.h>


#define MAC_ADDR_EQUAL(x, y)    \
    (x->w[0] == y->w[0]) && (x->w[1] == y->w[1]) && (x->w[2] == y->w[2])

/* Six-byte MAC address. NOTE: bytes stored in network order. */
typedef union mac_addr
{
    u8 b[6];
    u16 w[3];
} mac_addr_t;


/* Enumeration of Ethertype values */
typedef enum ethertype
{
    ethertype_unknown   = 0x0000,
    ethertype_ipv4      = 0x0800,
    ethertype_arp       = 0x0806
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
net_address_t *eth_make_addr(const mac_addr_t * const mac, net_address_t *addr);
net_address_t *eth_make_broadcast_addr(net_address_t *addr);
const mac_addr_t *eth_get_addr(const net_address_t * const addr);
s32 eth_addr_compare(const net_address_t * const a1, const net_address_t * const a2);
net_protocol_t eth_proto_from_ethertype(ku16 ethertype);
ethertype_t eth_ethertype_from_proto(const net_protocol_t proto);
s32 eth_print_addr(const net_address_t *addr, char *buf, s32 len);

#endif /* WITH_NET_ETHERNET */
#endif /* WITH_NETWORKING */
#endif
