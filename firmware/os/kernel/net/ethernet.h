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

extern mac_addr_t g_mac_zero;


/* Ethernet II frame header */
typedef struct eth_hdr
{
    mac_addr_t      dest;
    mac_addr_t      src;
    u16             type;
} eth_hdr_t;


/* Enumeration of Ethertype values */
typedef enum ethertype
{
    ethertype_ipv4  = 0x0800,
    ethertype_arp   = 0x0806
} ethertype_t;


void eth_handle_frame(net_iface_t *iface, void *frame, u32 len);

#endif
