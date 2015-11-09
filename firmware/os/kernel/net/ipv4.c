/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ipv4.h>
#include <klibc/stdio.h>        /* TODO remove */


void ip_print_addr(ipv4_addr_t addr)
{
    printf("%u.%u.%u.%u",
           ((addr >> 24) & 0xff), ((addr >> 16) & 0xff), ((addr >> 8) & 0xff), addr & 0xff);
}
