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
typedef struct net_iface net_iface_t;


typedef enum net_iface_type
{
    ni_ethernet
} net_iface_type_t;


typedef enum net_addr_type
{
    na_unknown = 0,
    na_ethernet,
    na_ipv4
} net_addr_type_t;


typedef enum net_protocol
{
    np_unknown,
    np_ethernet,
    np_arp,
    np_ipv4
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
#define NET_ADDR_LEN_ETHERNET           (6)     /* Ethernet (MAC) address   */
#define NET_ADDR_LEN_IPV4               (4)     /* IPv4 address             */

/* Set this to equal the length of the longest supported address type */
#define NET_MAX_ADDR_LEN                NET_ADDR_LEN_ETHERNET

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


/* Network protocol driver */
typedef struct net_proto_driver net_proto_driver_t;
struct net_proto_driver
{
    net_protocol_t          proto;
    const char *            name;
    s32                     (*rx)(net_packet_t *packet);
    s32                     (*tx)(net_iface_t *iface, net_addr_t *dest, ku16 proto,
                                  buffer_t *payload);
    s32                     (*reply)(net_packet_t *packet);
    net_proto_driver_t *    next;
};


/* Network interface. */
struct net_iface
{
    net_iface_t *       next;
    dev_t *             dev;                /* The hw device implementing this interface    */
    net_proto_driver_t *driver;             /* Protocol driver                              */
    net_iface_type_t    type;               /* Type of interface                            */
    net_address_t       hw_addr;            /* Hardware address                             */
    net_address_t       proto_addr;         /* Protocol address                             */
    net_iface_stats_t   stats;
};


/* Network packet object */
struct net_packet
{
    net_iface_t *           iface;
    net_protocol_t          proto;
    net_proto_driver_t *    driver;
    net_packet_t *          parent;
    buffer_t                raw;
};


s32 net_init();
net_proto_driver_t *net_get_proto_driver(const net_protocol_t proto);
s32 net_register_proto_driver(s32 (*init_fn)(net_proto_driver_t *));
net_iface_t *net_route_get(const net_addr_type_t addr_type, const net_addr_t *addr);
s32 net_alloc_packet(ku32 len, net_packet_t **packet);
void net_free_packet(net_packet_t *packet);
s32 net_transmit(net_packet_t *packet);
s16 net_cksum(const void *buf, u32 len);
s32 net_address_compare(const net_address_t *a1, const net_address_t *a2);
s32 net_print_addr(const net_address_t *addr, char *buf, s32 len);

#endif
