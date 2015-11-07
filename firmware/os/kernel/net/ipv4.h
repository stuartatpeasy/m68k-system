#ifndef KERNEL_NET_IPV4_H_INC
#define KERNEL_NET_IPV4_H_INC
/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <include/byteorder.h>
#include <include/defs.h>
#include <include/types.h>
#include <kernel/net/ethernet.h>


/* IPv4 packet header */
typedef struct ipv4_hdr
{
    u8              version_hdr_len;
    u8              diff_svcs;          /* Differentiated services field            */
    u16             total_len;
    u16             id;
    u16             flags_frag_offset;  /* Flags / fragment offset                  */
    u8              ttl;
    u8              protocol;
    u16             cksum;
    u32             src;
    u32             dest;
} ipv4_hdr_t;


// FIXME MOVE
/* UDP header */
typedef struct udp_hdr
{
    u16             src_port;
    u16             dest_port;
    u16             len;
    u16             cksum;
} udp_hdr_t;


/* Enumeration of IP protocol IDs */
typedef enum ipv4_protocol
{
    ip_proto_tcp    = 6,
    ip_proto_udp    = 17
} ipv4_protocol_t;

#endif
