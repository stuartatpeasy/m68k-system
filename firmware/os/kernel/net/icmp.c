/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <klibc/string.h>


s32 icmp_handle_echo_request(net_packet_t *packet, net_packet_t **reply);


/*
    icmp_handle_packet() - handle an incoming ICMP packet.
*/
s32 icmp_handle_packet(net_iface_t *iface, net_packet_t *packet, net_packet_t **reply)
{
    const icmp_fixed_hdr_t * const icmp_hdr = (const icmp_fixed_hdr_t *) packet->data;
    UNUSED(iface);

    /*
        It's usually not necessary to verify the ICMP checksum on received packets, as the hardware
        will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(ICMP_VERIFY_CHECKSUM)
    if(net_cksum(packet->data, packet->len) != 0x0000)
        return SUCCESS;     /* Drop packet */
#endif
    switch(icmp_hdr->type)
    {
        case icmp_echo_request:
            return icmp_handle_echo_request(packet, reply);
    }

    return SUCCESS;
}


/*
    icmp_handle_echo_request() - handle an incoming ICMP echo request packet.
*/
s32 icmp_handle_echo_request(net_packet_t *packet, net_packet_t **reply)
{
    s32 ret;
    icmp_echo_reply_t *r;

    if(((icmp_echo_request_t *) packet->data)->hdr.code != 0)    /* "code" field must be 0 */
        return SUCCESS;     /* drop packet */

    ret = net_packet_dup(packet, reply);
    if(ret != SUCCESS)
        return ret;

    r = (*reply)->data;
    r->hdr.checksum = 0;
    r->hdr.type = icmp_echo_reply;
    r->hdr.checksum = net_cksum(r, packet->len);

    return SUCCESS;
}
