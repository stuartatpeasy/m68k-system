#ifndef KERNEL_NET_NET_H_INC
#define KERNEL_NET_NET_H_INC
/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/util/buffer.h>


typedef struct net_packet net_packet_t;

typedef enum net_addr_type
{
    na_unknown = 0,
    na_ethernet,
    na_ipv4,
    na_ipv6,
    na_invalid
} net_addr_type_t;


typedef enum net_protocol
{
    np_unknown = 0,
    np_raw,                 /* Raw packets - no protocol wrapper    */
    np_ethernet,            /* Ethernet protocol                    */
    np_arp,                 /* Address Resolution Protocol          */
    np_ipv4,                /* IP v4                                */
    np_ipv6,                /* IP v6                                */
    np_tcp,                 /* TCP                                  */
    np_udp,                 /* UDP                                  */
    np_icmp,                /* ICMP (ping, etc.)                    */
    np_invalid              /* Used as a delimiter for validation   */
} net_protocol_t;


typedef struct net_iface_stats
{
    u32     rx_packets;
    u32     rx_bytes;
    u32     rx_checksum_err;
    u32     rx_dropped;
    u32     tx_packets;
    u32     tx_bytes;
} net_iface_stats_t;


/* Lengths of the various supported network address types */
#define NET_ADDR_LEN_ETHERNET           (6)     /* Ethernet (MAC) address       */
#define NET_ADDR_LEN_IPV4               (6)     /* IPv4 address, including port */

/* Set this to equal the length of the longest supported address type */
#define NET_MAX_ADDR_LEN                (6)

typedef union net_addr
{
    u8 addr;
    u8 addr_bytes[NET_MAX_ADDR_LEN];
} net_addr_t;


/* Composite network address, including address type */
typedef struct net_address
{
    net_addr_type_t type;
    net_addr_t      addr;
} net_address_t;


s32 net_init();
s32 net_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_tx_free(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s16 net_cksum(const void *buf, u32 len);
s32 net_address_compare(const net_address_t *a1, const net_address_t *a2);
net_addr_type_t net_address_get_type(const net_address_t * const addr);
s32 net_address_set_type(const net_addr_type_t type, net_address_t * const addr);
const void *net_address_get_address(const net_address_t * const addr);
net_protocol_t net_address_get_proto(const net_address_t * const addr);
net_protocol_t net_address_get_hwproto(const net_address_t * const addr);
s32 net_print_addr(const net_address_t * const addr, char * const buf, s32 len);

#endif
