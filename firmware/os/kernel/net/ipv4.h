#ifndef KERNEL_NET_IPV4_H_INC
#define KERNEL_NET_IPV4_H_INC
/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/net.h>


#define IPV4_SRC_ADDR_DEFAULT       ((ipv4_addr_t) 0)

#define IPV4_HDR_FLAG_DF            BIT(14)     /* Don't Fragment (DF) flag             */
#define IPV4_HDR_FLAG_MF            BIT(13)     /* More Fragments (MF) flag             */

#define IPV4_DEFAULT_TTL            (64)


/* IPv4 address */
typedef u32 ipv4_addr_t;


/* IPv4 protocol IDs */
typedef enum ipv4_protocol
{
    ipv4_proto_icmp     = 1,
    ipv4_proto_tcp      = 6,
    ipv4_proto_udp      = 17
} ipv4_protocol_t;


/* IPv4 packet header */
typedef struct ipv4_hdr
{
    u8              version_hdr_len;
    u8              diff_svcs;          /* Differentiated services field            */
    u16             total_len;
    u16             id;
    u16             flags_frag_offset;  /* Flags / fragment offset                  */
    u8              ttl;
    u8              protocol;
    u16             cksum;
    u32             src;
    u32             dest;
} ipv4_hdr_t;


typedef struct ipv4_route_ent
{
    ipv4_addr_t     dest;
    ipv4_addr_t     gateway;
    net_iface_t *   iface;
    u8              mask;
    u8              flags;
    u8              metric;
} ipv4_route_ent_t;


s32 ipv4_handle_packet(net_iface_t *iface, net_packet_t *packet, net_packet_t **response);
s32 ipv4_send_packet(const ipv4_addr_t src, const ipv4_addr_t dest, const ipv4_protocol_t proto,
                     const void *packet, u32 len);

#endif
