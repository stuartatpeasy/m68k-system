#ifndef KERNEL_INCLUDE_NET_NET_H_INC
#define KERNEL_INCLUDE_NET_NET_H_INC
/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/device/device.h>
#include <kernel/include/types.h>
#include <kernel/util/buffer.h>


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


/*
    Forward declarations of various types (an attempt to avoid circular-dependency hell)
*/
typedef struct net_packet net_packet_t;
typedef enum net_protocol net_protocol_t;
typedef struct net_address net_address_t;
typedef struct net_iface net_iface_t;

/* Network module (i.e. protocol driver) init function typedef */
typedef s32 (*net_init_fn_t)();


s32 net_init();
s32 net_tx(net_packet_t *packet);
s16 net_cksum(const void *buf, u32 len);
void net_receive(void *arg);

#endif
