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


typedef enum net_addr_type
{
    na_unknown = 0,
    na_ethernet,
    na_ipv4
} net_addr_type_t;


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

/* Network interface. */
typedef struct net_iface net_iface_t;

struct net_iface
{
    net_iface_t *   next;
    dev_t *         dev;                /* The hardware device implementing this interface      */
    net_addr_type_t hw_addr_type;       /* Type of the hardware address                         */
    net_addr_type_t proto_addr_type;    /* Type of the protocol address                         */
    net_addr_t      hw_addr;            /* Hardware address of the interface                    */
    net_addr_t      proto_addr;         /* Protocol address (e.g. IPv4 addr) of the interface   */
};


s32 net_init();
net_iface_t *net_route_get(const net_addr_type_t addr_type, const net_addr_t *addr);
s32 net_transmit(net_iface_t *iface, const void *buffer, u32 len);
s16 net_cksum(const void *buf, u32 len);

#endif
