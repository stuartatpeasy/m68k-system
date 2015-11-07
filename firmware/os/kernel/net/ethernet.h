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


void eth_handle_frame(void *frame, u32 len);


/* Six-byte MAC address. NOTE: bytes stored in network order. */
typedef struct mac_addr
{
    u8 b[6];
} mac_addr_t;


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

#endif
