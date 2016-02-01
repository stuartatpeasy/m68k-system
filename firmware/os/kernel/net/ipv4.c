/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>
#include <kernel/net/tcp.h>
#include <kernel/net/udp.h>


s32 ipv4_rx(net_packet_t *packet);
s32 ipv4_tx(net_iface_t *iface, net_addr_t *dest, ku16 type, buffer_t *payload);
s32 ipv4_reply(net_packet_t *packet);

s32 ipv4_send_packet(const ipv4_addr_t src, const ipv4_addr_t dest, const ipv4_protocol_t proto,
                     const void *packet, u32 len);


/*
    ipv4_init() - initialise the IPv4 protocol driver
*/
s32 ipv4_init(net_proto_driver_t *driver)
{
    driver->name = "IPv4";
    driver->proto = np_ipv4;
    driver->rx = ipv4_rx;
    driver->tx = ipv4_tx;
    driver->reply = ipv4_reply;

    return SUCCESS;
}


/*
    ipv4_handle_packet() - handle an incoming IPv4 packet.  Return EINVAL if the packet is invalid;
    returns ESUCCESS if the packet was successfully processed, or if the packet was ignored.
*/
s32 ipv4_rx(net_packet_t *packet)
{
    ipv4_hdr_t *hdr = (ipv4_hdr_t *) packet->raw.data;
    net_packet_t ipv4packet;
    s32 ret;

    /*
        It's usually not necessary to verify the IPv4 header checksum on received packets, as the
        hardware will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(IPV4_VERIFY_CHECKSUM)
    if(net_cksum(hdr, (hdr->version_hdr_len & 0xf) << 2) != 0x0000)
        return SUCCESS;     /* Drop packet */
#endif

    ipv4packet.iface = packet->iface;
    ipv4packet.proto = np_ipv4;
    ipv4packet.raw.len = packet->raw.len - sizeof(ipv4_hdr_t);
    ipv4packet.raw.data = (void *) &hdr[1];
    ipv4packet.driver = net_get_proto_driver(np_ipv4);
    ipv4packet.parent = packet;

    switch(hdr->protocol)
    {
        case ipv4_proto_icmp:
            ret = icmp_rx(&ipv4packet);
            break;

        case ipv4_proto_tcp:
            ret = tcp_rx(&ipv4packet);
            break;

        case ipv4_proto_udp:
            ret = udp_rx(&ipv4packet);
            break;

        default:
            return SUCCESS;     /* Drop packet */
    }

    if(ret != SUCCESS)
        return ret;
#if 0
    if(proto_response != NULL)
    {
        /* Encapsulate packet, free original response packet, and respond */
        net_packet_t *r;
        ipv4_hdr_t *rhdr;

        ret = net_packet_alloc(sizeof(ipv4_hdr_t) + proto_response->len, &r);
        if(ret != SUCCESS)
        {
            net_packet_free(proto_response);
            return ret;
        }

        rhdr = (ipv4_hdr_t *) r->data;

        rhdr->cksum             = 0;
        rhdr->version_hdr_len   = (4 << 4) | 5;     /* IPv4, hdr len = 5 32-bit words (=20 bytes) */
        rhdr->diff_svcs         = 0;
        rhdr->total_len         = sizeof(ipv4_hdr_t) + proto_response->len;
        rhdr->id                = rand();           /* FIXME - almost certainly wrong for pkt id  */
        rhdr->flags_frag_offset = IPV4_HDR_FLAG_DF;
        rhdr->ttl               = IPV4_DEFAULT_TTL;
        rhdr->protocol          = hdr->protocol;
        rhdr->src               = hdr->dest;
        rhdr->dest              = hdr->src;
        rhdr->cksum             = net_cksum(rhdr, sizeof(ipv4_hdr_t));


        memcpy(&rhdr[1], proto_response->data, proto_response->len);
        net_packet_free(proto_response);

        *response = r;
    }
#endif

    return SUCCESS;
}


/*
    ipv4_tx() - transmit an IPv4 packet
*/
s32 ipv4_tx(net_iface_t *iface, net_addr_t *dest, ku16 type, buffer_t *payload)
{
    UNUSED(iface); UNUSED(dest); UNUSED(type); UNUSED(payload);
    puts("ipv4_tx");
    return SUCCESS;
}


/*
    ipv4_reply() - reply to an IPv4 packet
*/
s32 ipv4_reply(net_packet_t *packet)
{
    ipv4_hdr_t *hdr = packet->raw.data;
    ipv4_addr_t tmp;

    tmp = hdr->src;
    hdr->src = hdr->dest;
    hdr->dest = tmp;

    return packet->parent->driver->reply(packet->parent);
}


/*
    ipv4_send_packet() - send an IPv4 packet to a peer
*/
s32 ipv4_send_packet(const ipv4_addr_t src, const ipv4_addr_t dest, const ipv4_protocol_t proto,
                     const void *packet, u32 len)
{
    ////////////////////// FIXME - reimplement this in terms of net_packet_t //////////////////
    UNUSED(src);
    UNUSED(dest);
    UNUSED(proto);
    UNUSED(packet);
    UNUSED(len);
#if 0

    ipv4_hdr_t *hdr;
    ipv4_addr_t srcaddr;
    void *buffer;
    u32 total_len;

    net_iface_t *iface = net_route_get(na_ipv4, (net_addr_t *) &dest);
    if(iface == NULL)
        return EHOSTUNREACH;

    total_len = sizeof(ipv4_hdr_t) + len;
    buffer = kmalloc(total_len);
    if(buffer == NULL)
        return ENOMEM;

    hdr = (ipv4_hdr_t *) buffer;

    srcaddr =
    (src == IPV4_SRC_ADDR_DEFAULT) ?
    *((ipv4_addr_t *) &iface->proto_addr) :
        src;

    /* TODO - fragmentation... */

    /* Create an IPv4 header for the packet */
    hdr->version_hdr_len    = (4 << 4) | 5;     /* IPv4, header len = 5 32-bit words (=20 bytes) */
    hdr->diff_svcs          = 0;
    hdr->total_len          = sizeof(ipv4_hdr_t) + len;
    hdr->id                 = rand();           /* FIXME - rand() almost certainly wrong for pkt id */
    hdr->flags_frag_offset  = IPV4_HDR_FLAG_DF;
    hdr->ttl                = 64;               /* Sensible default? */
    hdr->protocol           = proto;
    hdr->src                = srcaddr;
    hdr->dest               = dest;

    memcpy((u8 *) buffer + sizeof(ipv4_hdr_t), packet, len);

    return net_transmit(iface, buffer, total_len);
#endif
    return SUCCESS;
}
