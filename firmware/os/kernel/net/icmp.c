/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <klibc/string.h>


s32 icmp_handle_echo_request(net_iface_t *iface, const icmp_echo_request_t *packet, u32 len);


/*
    icmp_handle_packet() - handle an incoming ICMP packet.
*/
s32 icmp_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    const icmp_fixed_hdr_t * const icmp_hdr = (const icmp_fixed_hdr_t *) packet;
    UNUSED(iface);

    /*
        It's usually not necessary to verify the ICMP checksum on received packets, as the hardware
        will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(ICMP_VERIFY_CHECKSUM)
    if(net_cksum(packet, len) != 0x0000)
        return SUCCESS;     /* Drop packet */
#endif
    switch(icmp_hdr->type)
    {
        case icmp_echo_request:
            return icmp_handle_echo_request(iface, packet, len);
    }

    return SUCCESS;
}


/*
    icmp_handle_echo_request() - handle an incoming ICMP echo request packet.
*/
s32 icmp_handle_echo_request(net_iface_t *iface, const icmp_echo_request_t *packet, u32 len)
{
    icmp_echo_reply_t *reply;
    UNUSED(iface);

    if(packet->hdr.code != 0)   /* "code" field must be 0 */
        return SUCCESS;         /* drop packet */

    reply = (icmp_echo_reply_t *) kmalloc(len);
    if(!reply)
        return ENOMEM;

    memcpy(reply, packet, len);
    reply->hdr.type = icmp_echo_reply;

    /* TODO: work out how to send the packet back up through the protocol layer */
    puts("ping");

    return SUCCESS;
}
