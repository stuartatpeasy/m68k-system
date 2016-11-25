#ifndef KERNEL_NET_ADDRESS_H_INC
#define KERNEL_NET_ADDRESS_H_INC
/*
    Network address abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/net/net.h>

/* Lengths of the various supported network address types */
#define NET_ADDR_LEN_ETHERNET           (6)     /* Ethernet (MAC) address       */
#define NET_ADDR_LEN_IPV4               (6)     /* IPv4 address, including port */

/* Set this to equal the length of the longest supported address type */
#define NET_MAX_ADDR_LEN                (6)

typedef enum net_addr_type
{
    na_unknown = 0,
    na_ethernet,
    na_ipv4,
    na_ipv6,
    na_invalid
} net_addr_type_t;


/* Storage for a network address as raw bytes */
typedef union net_addr
{
    u8 addr;
    u8 addr_bytes[NET_MAX_ADDR_LEN];
} net_addr_t;


/* Composite network address, including address type */
typedef struct net_address
{
    net_addr_t      addr;   /* Needs to be first member in order to ensure alignment */
    net_addr_type_t type;
} net_address_t;


s32 net_address_compare(const net_address_t *a1, const net_address_t *a2);
net_addr_type_t net_address_get_type(const net_address_t * const addr);
s32 net_address_set_type(const net_addr_type_t type, net_address_t * const addr);
const void *net_address_get_address(const net_address_t * const addr);
net_protocol_t net_address_get_proto(const net_address_t * const addr);
net_protocol_t net_address_get_hw_proto(const net_address_t * const addr);
s32 net_address_print(const net_address_t * const addr, char * const buf, s32 len);
net_addr_type_t net_address_type_from_proto(const net_protocol_t proto);

#endif
