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
#include <kernel/net/address.h>


/*
    Forward declarations of various types (an attempt to avoid circular-dependency hell)
*/
typedef struct net_packet net_packet_t;
typedef enum net_protocol net_protocol_t;


typedef struct net_iface_stats
{
    u32     rx_packets;
    u32     rx_bytes;
    u32     rx_checksum_err;
    u32     rx_dropped;
    u32     tx_packets;
    u32     tx_bytes;
} net_iface_stats_t;

/* Network module (i.e. protocol driver) init function typedef */
typedef s32 (*net_init_fn_t)();


s32 net_init();
s32 net_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_tx_free(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s16 net_cksum(const void *buf, u32 len);
void net_receive(void *arg);

#endif
