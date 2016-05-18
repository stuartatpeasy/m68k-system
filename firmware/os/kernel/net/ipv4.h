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
#include <kernel/net/address.h>
#include <kernel/net/interface.h>
#include <kernel/net/protocol.h>


#define IPV4_ADDR_NONE              ((ipv4_addr_t) 0)
#define IPV4_ADDR_BROADCAST         ((ipv4_addr_t) 0xffffffff)
#define IPV4_MASK_HOST_ONLY         ((ipv4_addr_t) 0xffffffff)
#define IPV4_SRC_ADDR_DEFAULT       IPV4_ADDR_NONE
#define IPV4_PORT_NONE              ((ipv4_port_t) 0)

#define IPV4_HDR_FLAG_DF            BIT(14)     /* Don't Fragment (DF) flag             */
#define IPV4_HDR_FLAG_MF            BIT(13)     /* More Fragments (MF) flag             */

#define IPV4_DEFAULT_TTL            (64)


/* IPv4 address */
typedef u32 ipv4_addr_t;

/* IPv4 port number */
typedef u16 ipv4_port_t;

/* IPv4 address/port combination */
typedef struct ipv4_address
{
    ipv4_addr_t     addr;
    ipv4_port_t     port;
} ipv4_address_t;

/* IPv4 protocol IDs */
typedef enum ipv4_protocol
{
    ipv4_proto_icmp     = 1,
    ipv4_proto_tcp      = 6,
    ipv4_proto_udp      = 17,
    ipv4_proto_invalid  = 255
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


s32 ipv4_init();
s32 ipv4_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                      net_packet_t **packet);
net_address_t *ipv4_make_addr(const ipv4_addr_t ip, const ipv4_port_t port, net_address_t *addr);
net_address_t *ipv4_make_broadcast_addr(net_address_t * const addr);
s32 ipv4_rx(net_packet_t *packet);
s32 ipv4_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
ipv4_addr_t ipv4_get_addr(const net_address_t * const addr);
ipv4_port_t ipv4_get_port(const net_address_t * const addr);
s32 ipv4_addr_compare(const net_address_t * const a1, const net_address_t * const a2);
s32 ipv4_print_addr(const net_address_t *addr, char *buf, s32 len);

#endif
