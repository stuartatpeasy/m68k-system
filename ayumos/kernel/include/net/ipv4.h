#ifndef KERNEL_INCLUDE_NET_IPV4_H_INC
#define KERNEL_INCLUDE_NET_IPV4_H_INC
/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING
#ifdef WITH_NET_IPV4

#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/net.h>
#include <kernel/include/net/address.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/protocol.h>


#define IPV4_ADDR_NONE              ((ipv4_addr_t) 0)
#define IPV4_ADDR_BROADCAST         ((ipv4_addr_t) 0xffffffff)
#define IPV4_MASK_NONE              ((ipv4_addr_t) 0)
#define IPV4_MASK_HOST_ONLY         ((ipv4_addr_t) 0xffffffff)
#define IPV4_SRC_ADDR_DEFAULT       IPV4_ADDR_NONE
#define IPV4_PORT_NONE              ((ipv4_port_t) 0)
#define IPV4_PREFIX_LEN_MAX         (32)

#define IPV4_HDR_FLAG_DF            BIT(14)     /* Don't Fragment (DF) flag             */
#define IPV4_HDR_FLAG_MF            BIT(13)     /* More Fragments (MF) flag             */

#define IPV4_DEFAULT_TTL            (64)        /* Is this a sensible default?          */


/* Definitions relating to port types */
#ifndef IPV4_EPHEM_PORT_START
#define IPV4_EPHEM_PORT_START       (49152)     /* First ephemeral port number          */
#endif
#ifndef IPV4_EPHEM_PORT_END
#define IPV4_EPHEM_PORT_END         (65536)     /* Last ephemeral port number + 1       */
#endif

/* Privileged ports: the first is always port 0; the last is configurable. */
#ifndef IPV4_PRIV_PORT_END
#define IPV4_PRIV_PORT_END          (1024)      /* Last privileged port + 1             */
#endif

#ifndef IPV4_GENERAL_PORT_START
#define IPV4_GENERAL_PORT_START     (1024)      /* First general (unprivileged) port    */
#endif
#ifndef IPV4_GENERAL_PORT_END
#define IPV4_GENERAL_PORT_END       (49152)     /* Last general (unprivileged) port + 1 */
#endif

/* Constants used to specify a port type to ipv4_port_alloc() */
#define IPV4_PORT_NUM_SPECIFIC      (0)
#define IPV4_PORT_NUM_EPHEMERAL     (1)

/*
    IPV4_PORTS_PER_BITMAP specifies the number of port-numbers covered by a bitmap the size of a
    slab.
*/
#define IPV4_PORTS_PER_BITMAP       (SLAB_MAX_SIZE * 8)

/*
    IPV4_PORTS_PER_SLAB specifies the number of port-numbers contained within one slab's worth of
    port-allocation bitmaps.
*/
#define IPV4_PORTS_PER_SLAB         (IPV4_PORTS_PER_BITMAP * (SLAB_MAX_SIZE / sizeof(u8 *)))

/* Routing table flags */
#define IPV4_ROUTE_UP           BIT(0)      /* Route is up (active)             */
#define IPV4_ROUTE_HOST         BIT(1)      /* Target is a host                 */
#define IPV4_ROUTE_GATEWAY      BIT(2)      /* Use a gateway                    */
#define IPV4_ROUTE_REJECT       BIT(3)      /* A "reject" route - drop traffic  */

#define IPV4_ROUTE_METRIC_MIN   (0)         /* Minimum allowable route metric   */
#define IPV4_ROUTE_METRIC_MAX   U16_MAX     /* Maximum allowable route metric   */


/* IPv4 address */
typedef u32 ipv4_addr_t;

/* IPv4 port number */
typedef u16 ipv4_port_t;

/* IPv4 port-allocation bitmap */
typedef u8 *** ipv4_port_alloc_bitmap_t;

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


/* An IPv4 route */
typedef struct ipv4_route ipv4_route_t;

struct ipv4_route
{
    net_iface_t     *iface;
    ipv4_addr_t     dest;
    ipv4_addr_t     mask;
    ipv4_addr_t     gateway;
    s16             metric;
    u16             flags;
    s16             prefix_len;
};


/* IPv4 routing table entry */
typedef struct ipv4_rt_item ipv4_rt_item_t;

struct ipv4_rt_item
{
    ipv4_route_t    r;
    ipv4_rt_item_t  *next;
};


s32 ipv4_init();
s32 ipv4_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                      net_packet_t **packet);
net_address_t *ipv4_make_addr(const ipv4_addr_t ip, const ipv4_port_t port, net_address_t *addr);
ipv4_address_t *ipv4_addr_set_port(ipv4_address_t * const addr, const ipv4_port_t port);
net_address_t *ipv4_make_broadcast_addr(net_address_t * const addr);
s32 ipv4_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 ipv4_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
ipv4_addr_t ipv4_get_addr(const net_address_t * const addr);
ipv4_port_t ipv4_get_port(const net_address_t * const addr);
s32 ipv4_addr_compare(const net_address_t * const a1, const net_address_t * const a2);
s32 ipv4_print_addr(const net_address_t *addr, char *buf, s32 len);
u32 ipv4_mask_valid(const ipv4_addr_t mask);
u8 ipv4_mask_to_prefix_len(const ipv4_addr_t mask);

s32 ipv4_route_add(const ipv4_route_t * const r);
s32 ipv4_route_delete(const ipv4_route_t * const r);
s32 ipv4_route_get_entry(ipv4_rt_item_t **e);
const ipv4_route_t *ipv4_route_get(const net_address_t * const proto_addr);
s32 ipv4_route_get_iface(const net_address_t * const proto_addr, net_iface_t **iface);
s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr);

s32 ipv4_port_alloc_bitmap_init(ipv4_port_alloc_bitmap_t *bitmap);
s32 ipv4_port_alloc_bitmap_free(ipv4_port_alloc_bitmap_t bitmap);
s32 ipv4_port_alloc(ipv4_port_alloc_bitmap_t alloc_bitmap, ipv4_port_t *port, ku16 type);
s32 ipv4_port_free(ipv4_port_alloc_bitmap_t alloc_bitmap, const ipv4_port_t port);

#endif /* WITH_NET_IPV4 */
#endif /* WITH_NETWORKING */
#endif
