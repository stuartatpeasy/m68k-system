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
#include <kernel/net/address.h>


#define NET_INTERFACE_ANY           ((net_iface_t *) NULL)


/* Network interface. */
typedef struct net_iface net_iface_t;


s32 net_interface_init();
s32 net_interface_rx(net_iface_t * const iface, net_packet_t *packet);
net_iface_t *net_interface_get_by_dev(const char * const name);
const char *net_get_iface_name(const net_iface_t * const iface);
dev_t *net_interface_get_device(net_iface_t * const iface);
const net_address_t *net_interface_get_proto_addr(const net_iface_t * const iface);
s32 net_interface_set_proto_addr(net_iface_t * const iface, const net_address_t * const addr);
const net_address_t *net_interface_get_hw_addr(const net_iface_t * const iface);
net_protocol_t net_interface_get_proto(const net_iface_t * const iface);
void net_interface_stats_inc_rx_packets(net_iface_t * const iface);
void net_interface_stats_add_rx_bytes(net_iface_t * const iface, ku32 bytes);
void net_interface_stats_inc_tx_packets(net_iface_t * const iface);
void net_interface_stats_add_tx_bytes(net_iface_t * const iface, ku32 bytes);
void net_interface_stats_inc_cksum_err(net_iface_t * const iface);
void net_interface_stats_inc_rx_dropped(net_iface_t * const iface);

#endif
