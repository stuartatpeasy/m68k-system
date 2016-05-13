#ifndef KERNEL_NET_PROTOCOL_H_INC
#define KERNEL_NET_PROTOCOL_H_INC
/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/net/net.h>
#include <kernel/net/packet.h>



/* Network protocol driver */
typedef struct net_proto_driver net_proto_driver_t;
struct net_proto_driver
{
    net_protocol_t          proto;
    const char *            name;
    s32                     (*rx)(net_packet_t *packet);
    s32                     (*tx)(const net_address_t *src, const net_address_t *dest,
                                  net_packet_t *packet);
    net_proto_driver_t *    next;
};

/* Network protocol driver init function typedef */
typedef s32 (*net_protocol_init_fn_t)(net_proto_driver_t *);


s32 net_protocol_init();
net_proto_driver_t *net_get_proto_driver(const net_protocol_t proto);
net_proto_driver_t *net_protocol_get_driver(const net_protocol_t proto);

#endif
