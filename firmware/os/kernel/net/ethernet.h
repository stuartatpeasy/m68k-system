#ifndef KERNEL_NET_ETHERNET_H_INC
#define KERNEL_NET_ETHERNET_H_INC
/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <include/byteorder.h>
#include <include/defs.h>
#include <include/types.h>
#include <kernel/device/device.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>


/* Six-byte MAC address. NOTE: bytes stored in network order. */
typedef union mac_addr
{
    u8 b[6];
    u16 w[3];
} mac_addr_t;

mac_addr_t g_mac_broadcast;


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


s32 eth_handle_packet(net_iface_t *iface, const void *packet, u32 len);
s32 eth_transmit(net_iface_t *iface, const mac_addr_t *dest, const ethertype_t et, void *packet,
                 u32 len);

#endif
