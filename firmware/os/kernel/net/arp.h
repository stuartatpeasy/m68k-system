#ifndef KERNEL_NET_ARP_H_INC
#define KERNEL_NET_ARP_H_INC
/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/error.h>
#include <kernel/include/types.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>


#define ARP_CACHE_SIZE              (16)        /* Number of slots in ARP cache                 */
#define ARP_CACHE_ITEM_LIFETIME     (1200)      /* Lifetime of an ARP cache item in seconds     */
#define ARP_MAX_REQUESTS            (5)         /* Max num of consecutive requests for an addr  */
#define ARP_REQUEST_INTERVAL        (2)         /* #secs between ARP requests for a given addr  */

s32 arp_init();
s32 arp_handle_packet(net_iface_t *iface, const void * const packet, u32 len);
s32 arp_lookup_ip(const ipv4_addr_t ip, net_iface_t *iface, mac_addr_t *hw_addr);


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
    const net_iface_t * iface;
    mac_addr_t          hw_addr;
    u32                 ipv4_addr;
    s32                 etime;
} arp_cache_item_t;

#endif
