#ifndef KERNEL_NET_ETHERNET_H_INC
#define KERNEL_NET_ETHERNET_H_INC
/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <device/device.h>
#include <include/byteorder.h>
#include <include/defs.h>
#include <include/types.h>
#include <kernel/net/ipv4.h>


/* Six-byte MAC address. NOTE: bytes stored in network order. */
typedef struct mac_addr
{
    u8 b[6];
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


/* Ethernet interface */
typedef struct eth_iface
{
    dev_t           *dev;       /* The hardware device implementing this interface      */
    mac_addr_t      hw_addr;    /* Hardware address of the interface                    */
    ipv4_addr_t     ipv4_addr;  /* IPv4 address associated with this interface          */

} eth_iface_t;


void eth_handle_frame(eth_iface_t *iface, void *frame, u32 len);

#endif
