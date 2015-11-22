/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/udp.h>
#include <klibc/stdio.h>        /* TODO remove */


void ip_print_addr(ipv4_addr_t addr)
{
    printf("%u.%u.%u.%u",
           ((addr >> 24) & 0xff), ((addr >> 16) & 0xff), ((addr >> 8) & 0xff), addr & 0xff);
}


/*
    ipv4_handle_packet() - handle an incoming IPv4 packet.  Return EINVAL if the packet is invalid;
    returns ESUCCESS if the packet was successfully processed, or if the packet was ignored.
*/
s32 ipv4_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    ipv4_hdr_t *hdr = (ipv4_hdr_t *) packet;
    void *payload = (void *) ((u8 *) packet + sizeof(ipv4_hdr_t));

    UNUSED(iface);
    UNUSED(len);
    UNUSED(hdr);
    UNUSED(payload);

    return SUCCESS;
}
