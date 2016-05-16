/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/packet.h>


s32 icmp_handle_echo_request(net_packet_t *packet);


/*
    icmp_rx() - handle an incoming ICMP packet.
*/
s32 icmp_rx(net_iface_t *iface, net_packet_t *packet)
{
    const icmp_fixed_hdr_t * const icmp_hdr =
        (const icmp_fixed_hdr_t *) net_packet_get_start(packet);

    /*
        It's usually not necessary to verify the ICMP checksum on received packets, as the hardware
        will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(ICMP_VERIFY_CHECKSUM)
    if(net_cksum(net_packet_get_start(packet), net_packet_get_len(packet)) != 0x0000)
    {
        net_interface_stats_inc_cksum_err(iface);
        net_interface_stats_inc_dropped(iface);
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
    icmp_echo_reply_t *r = net_packet_get_start(packet);

    if(((icmp_echo_request_t *) r)->hdr.code != 0)    /* "code" field must be 0 */
        return SUCCESS;     /* drop packet */

    r->hdr.checksum = 0;
    r->hdr.type     = icmp_echo_reply;
    r->hdr.checksum = net_cksum(r, packet->len);

// FIXME
//    return packet->driver->reply(packet);
return SUCCESS;
}
