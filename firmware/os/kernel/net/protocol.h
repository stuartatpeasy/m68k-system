#ifndef KERNEL_NET_PROTOCOL_H_INC
#define KERNEL_NET_PROTOCOL_H_INC
/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/net/net.h>
#include <kernel/net/address.h>
#include <kernel/net/interface.h>


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


/* Network protocol driver */
typedef struct net_proto_driver net_proto_driver_t;

typedef s32 (*net_rx_fn)(net_packet_t *packet);
typedef s32 (*net_tx_fn)(const net_address_t *src, const net_address_t *dest,
                           net_packet_t *packet);
typedef s32 (*net_addr_compare_fn)(const net_address_t * const a1, const net_address_t * const a2);


s32 net_protocol_register_driver(const net_protocol_t proto, const char * const name,
                                 net_rx_fn rx, net_tx_fn tx, net_addr_compare_fn addr_compare);
net_proto_driver_t *net_protocol_get_driver(const net_protocol_t proto);
net_protocol_t net_protocol_from_address(const net_address_t * const addr);
net_protocol_t net_protocol_hwproto_from_address(const net_address_t * const addr);
s32 net_protocol_addr_compare(const net_protocol_t proto, const net_address_t * const a1,
                              const net_address_t * const a2);
s32 net_protocol_rx(net_packet_t * const packet);

#endif
