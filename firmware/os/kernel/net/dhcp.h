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

#define BOOTP_FLAG_BROADCAST        BIT(15)


/* BOOTP packet-operation identifier */
typedef enum bootp_op
{
    bo_request  = 1,
    bo_reply    = 2
} bootp_op_t;


/* BOOTP hardware types (incomplete list) */
typedef enum bootp_htype
{
    bh_ethernet         = 1,
    bh_ieee802          = 6,
    bh_arcnet           = 7,
    bh_localtalk        = 11,
    bh_localnet         = 12,
    bh_smds             = 14,
    bh_frame_relay      = 15,
    bh_atm              = 16,
    bh_hdlc             = 17,
    bh_fibre_channel    = 18,
    bh_serial           = 20
} bootp_htype_t;


typedef enum dhcp_msg_type
{
    dmt_none        = 0,
    dmt_discover    = 1,
    dmt_offer       = 2,
    dmt_request     = 3,
    dmt_ack         = 5
} dhcp_msg_type_t;


/* DHCP packet format */
typedef struct dhcp_msg
{
    u8          op;             /* Operation - see bootp_op_t                                   */
    u8          htype;          /* Hardware type - see bootp_htype_t                            */
    u8          hlen;           /* Hardware address len, e.g. 6 for a MAC address               */
    u8          hops;           /* Hop count - controls forwarding of requests                  */
    u32         xid;            /* Transaction ID                                               */
    u16         secs;           /* Number of seconds since client attempted to acquire/renew    */
    u16         flags;          /* RFC 1542 flags - only contains "broadcast" flag              */
    ipv4_addr_t ciaddr;         /* Client IP address                                            */
    ipv4_addr_t yiaddr;         /* "Your" IP address                                            */
    ipv4_addr_t siaddr;         /* Server IP address                                            */
    ipv4_addr_t giaddr;         /* Gateway IP address                                           */
    u8          chaddr[16];     /* Client hardware address                                      */
    u8          zeroes[192];    /* Whole bunch o' zeroes                                        */
    u32         magic_cookie;   /* The DHCP magic cookie                                        */
} dhcp_msg_t;


/* DHCP option identifiers */
typedef enum dhcp_option_id
{
    dopt_padding                = 0,
    dopt_subnet_mask            = 1,
    dopt_router                 = 3,
    dopt_domain_name_server     = 6,
    dopt_domain_name            = 15,
    dopt_broadcast_addr         = 28,
    dopt_ip_addr_lease_time     = 51,
    dopt_msg_type               = 53,
    dopt_server_identifier      = 54,
    dopt_renewal_time_value     = 58,
    dopt_rebind_time_value      = 59,
    dopt_proxy_autodiscovery    = 252,
    dopt_end                    = 255
} dhcp_option_id_t;


/* DHCP option header */
typedef struct dhcp_option_hdr
{
    u8  id;
    u8  len;
} __attribute__((packed)) dhcp_option_hdr_t;

/* DHCP option field */
typedef struct dhcp_option
{
    dhcp_option_hdr_t   hdr;
    u8                  data;
} __attribute__((packed)) dhcp_option_t;


/* Struct to track the status of a DHCP request */
typedef struct dhcp_client_status
{
    u32     xid;            /* Transaction ID       */

} dhcp_client_status_t;


s32 dhcp_discover(net_iface_t *iface);
s32 dhcp_rx(net_iface_t *iface, net_packet_t *packet);

#endif
