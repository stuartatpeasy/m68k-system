#ifndef KERNEL_NET_ICMP_H_INC
#define KERNEL_NET_ICMP_H_INC
/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <klibc/string.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/net.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/ipv4.h>


/* Incomplete list of ICMP message types */
typedef enum icmp_type
{
    icmp_echo_reply             = 0,
    icmp_dest_unreachable       = 3,
    icmp_src_quench             = 4,
    icmp_redirect_message       = 5,
    icmp_echo_request           = 8,
    icmp_router_advertisement   = 9,
    icmp_router_solicitation    = 10,
    icmp_time_exceeded          = 11,
    icmp_parameter_problem      = 12,
    icmp_timestamp              = 13,
    icmp_timestamp_reply        = 14
} icmp_type_t;


/* The fixed part of the ICMP header */
typedef struct icmp_fixed_hdr
{
    u8      type;       /* Uses u8 in order to fix field size */
    u8      code;
    u16     checksum;
} icmp_fixed_hdr_t;


typedef struct icmp_echo_request
{
    icmp_fixed_hdr_t    hdr;
    u16                 id;
    u16                 seq;
    u8                  payload;
} icmp_echo_request_t;

typedef icmp_echo_request_t icmp_echo_reply_t;

s32 icmp_init();
s32 icmp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);

#endif
