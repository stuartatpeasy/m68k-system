#ifndef KERNEL_NET_ARP_H_INC
#define KERNEL_NET_ARP_H_INC
/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <include/byteorder.h>
#include <include/defs.h>
#include <include/error.h>
#include <include/types.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>


s32 arp_process_reply(const void * const packet, u32 len);
s32 arp_cache_add(const mac_addr_t hw_addr, const ipv4_addr_t ip);


typedef struct arp_hdr
{
    u16             hw_type;
    u16             proto_type;
    u8              hw_addr_len;
    u8              proto_addr_len;
    u16             opcode;
} arp_hdr_t;


/* ARP packet payload (Ethernet + IPv4 addresses only) */
typedef struct arp_payload
{
    mac_addr_t      src_mac;
    ipv4_addr_t     src_ip;
    mac_addr_t      dst_mac;
    ipv4_addr_t     dst_ip;
} arp_payload_t;


typedef struct arp_eth_ipv4_packet
{
    arp_hdr_t       hdr;
    arp_payload_t   payload;
} arp_eth_ipv4_packet_t;


/* ARP operation codes */
typedef enum arp_opcode
{
    arp_request             = 1,
    arp_reply               = 2
} arp_opcode_t;


/* ARP hardware address types */
typedef enum arp_hw_type
{
    arp_hw_type_ethernet    = 1
} arp_hw_type_t;


/* ARP cache entry */
typedef struct arp_cache_item
{
    mac_addr_t  hw_addr;
    u32         ipv4_addr;
    s32         etime;
} arp_cache_item_t;

#endif
