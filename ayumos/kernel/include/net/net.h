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
