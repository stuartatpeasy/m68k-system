/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <klibc/string.h>


s32 icmp_handle_echo_request(net_packet_t *packet);


/*
    icmp_rx() - handle an incoming ICMP packet.
*/
s32 icmp_rx(net_packet_t *packet)
{
    const icmp_fixed_hdr_t * const icmp_hdr = (const icmp_fixed_hdr_t *) packet->start;

    /*
        It's usually not necessary to verify the ICMP checksum on received packets, as the hardware
        will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(ICMP_VERIFY_CHECKSUM)
    if(net_cksum(packet->start, packet->len) != 0x0000)
    {
        ++packet->iface->stats.rx_checksum_err;
        ++packet->iface->stats.rx_dropped;
        return SUCCESS;     /* Drop packet */
    }
#endif

    switch(icmp_hdr->type)
    {
        case icmp_echo_request:
            return icmp_handle_echo_request(packet);
    }

    return SUCCESS;
}


/*
    icmp_handle_echo_request() - handle an incoming ICMP echo request packet.
*/
s32 icmp_handle_echo_request(net_packet_t *packet)
{
    icmp_echo_reply_t *r = packet->start;

    if(((icmp_echo_request_t *) packet->start)->hdr.code != 0)    /* "code" field must be 0 */
        return SUCCESS;     /* drop packet */

    r->hdr.checksum = 0;
    r->hdr.type     = icmp_echo_reply;
    r->hdr.checksum = net_cksum(r, packet->len);

    return packet->driver->reply(packet);
}
