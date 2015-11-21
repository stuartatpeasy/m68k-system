#ifndef KERNEL_NET_UDP_H_INC
#define KERNEL_NET_UDP_H_INC
/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <include/defs.h>
#include <include/types.h>


/* UDP header */
typedef struct udp_hdr
{
    u16             src_port;
    u16             dest_port;
    u16             len;
    u16             cksum;
} udp_hdr_t;

#endif
