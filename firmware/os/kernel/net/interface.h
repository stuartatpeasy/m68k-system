#ifndef KERNEL_NET_INTERFACE_H_INC
#define KERNEL_NET_INTERFACE_H_INC
/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/include/types.h>
#include <kernel/device/device.h>
#include <kernel/net/net.h>


#define NET_INTERFACE_ANY           ((net_iface_t *) NULL)


/* Network interface. */
typedef struct net_iface net_iface_t;
struct net_iface
{
    net_iface_t *       next;
    dev_t *             dev;            /* The hw device implementing this interface    */
    net_iface_type_t    type;           /* Type of interface                            */
    net_address_t       hw_addr;        /* Hardware address                             */
    net_address_t       proto_addr;     /* Protocol address                             */
    net_iface_stats_t   stats;
};


s32 net_interface_init();
net_iface_t *net_get_iface_by_dev(const char * const name);
const char *net_get_iface_name(const net_iface_t * const iface);
const net_address_t *net_get_proto_addr(const net_iface_t * const iface);
s32 net_set_proto_addr(net_iface_t * const iface, const net_address_t * const addr);
s32 net_interface_hw_addr_broadcast(net_iface_t * const iface, net_address_t * const addr);
void net_receive(void *arg);

#endif
