#ifndef KERNEL_NET_DHCP_H_INC
#define KERNEL_NET_DHCP_H_INC
/*
    DHCP client implementation

    Part of ayumos


    (c) Stuart Wallace, February 2016
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>
#include <kernel/net/udp.h>


#define DHCP_MAGIC_COOKIE           (0x63825363)
#define DHCP_DISCOVER_BUFFER_LEN    (512)

#define DHCP_SERVER_PORT            ((ipv4_port_t) 67)
#define DHCP_CLIENT_PORT            ((ipv4_port_t) 68)

/* BOOTP packet-operation identifier */
typedef enum bootp_op
{
    bo_request  = 1,
    bo_reply    = 2
} bootp_op_t;


typedef enum dhcp_msg_type
{
    dmt_discover    = 1,
    dmt_offer       = 2,
    dmt_request     = 3,
    dmt_ack         = 5
} dhcp_msg_type_t;


typedef enum dhcp_option
{
    do_msg_type     = 53,
    do_end          = 255
} dhcp_option_t;



/* DHCPDISCOVER message */
typedef struct dhcpdiscover_msg
{
    u8          op;
    u8          htype;
    u8          hlen;
    u8          hops;
    u32         xid;
    u16         secs;
    u16         flags;
    ipv4_addr_t ciaddr;
    ipv4_addr_t yiaddr;
    ipv4_addr_t siaddr;
    ipv4_addr_t giaddr;
    u8          chaddr[16];
    u8          zeroes[192];
    u32         magic_cookie;
} dhcpdiscover_msg_t;

#endif
