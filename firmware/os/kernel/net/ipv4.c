/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/ipv4route.h>
#include <kernel/net/net.h>
#include <kernel/net/tcp.h>
#include <kernel/net/udp.h>
#include <klibc/stdio.h>
#include <klibc/strings.h>


s32 ipv4_reply(net_packet_t *packet);
ipv4_protocol_t ipv4_get_proto(const net_protocol_t proto);

const net_address_t g_ipv4_broadcast =
{
    .type = na_ipv4,
    .addr.addr_bytes[0] = 0xff,
    .addr.addr_bytes[1] = 0xff,
    .addr.addr_bytes[2] = 0xff,
    .addr.addr_bytes[3] = 0xff
};


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
    ipv4_hdr_t *hdr = (ipv4_hdr_t *) packet->start;

    /*
        It's usually not necessary to verify the IPv4 header checksum on received packets, as the
        hardware will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(IPV4_VERIFY_CHECKSUM)
    if(net_cksum(hdr, (hdr->version_hdr_len & 0xf) << 2) != 0x0000)
        return SUCCESS;     /* Drop packet */
#endif

    packet->start += sizeof(ipv4_hdr_t);
    packet->len -= sizeof(ipv4_hdr_t);
    packet->proto = np_ipv4;
    packet->driver = net_get_proto_driver(np_ipv4);

    switch(hdr->protocol)
    {
        case ipv4_proto_icmp:
            return icmp_rx(packet);

        case ipv4_proto_tcp:
            return tcp_rx(packet);

        case ipv4_proto_udp:
            return udp_rx(packet);

        default:
            return SUCCESS;     /* Drop packet */
    }
}


/*
    ipv4_tx() - transmit an IPv4 packet
*/
s32 ipv4_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    ipv4_hdr_t *hdr;
    const ipv4_address_t *src_addr, *dest_addr;
    net_address_t dest_hw_addr;
    s32 ret;

    if(src == NULL)
    {
        /* FIXME - check that interface is actually configured */
        src = &packet->iface->proto_addr;
    }

    packet->start -= sizeof(ipv4_hdr_t);
    packet->len += sizeof(ipv4_hdr_t);

    hdr = (ipv4_hdr_t *) packet->start;
    src_addr = (ipv4_address_t *) &src->addr;
    dest_addr = (ipv4_address_t *) &dest->addr;

    hdr->version_hdr_len    = (4 << 4) | 5;     /* IPv4, header len = 5 32-bit words (=20 bytes) */
    hdr->diff_svcs          = 0;
    hdr->total_len          = packet->len;
    hdr->id                 = rand();           /* FIXME - rand() almost certainly wrong for pkt id */
    hdr->flags_frag_offset  = IPV4_HDR_FLAG_DF;
    hdr->ttl                = 64;               /* Sensible default? */
    hdr->protocol           = ipv4_get_proto(packet->proto);
    hdr->src                = src_addr->addr;
    hdr->dest               = dest_addr->addr;
    hdr->cksum              = 0x0000;

    hdr->cksum = net_cksum(packet->start, sizeof(ipv4_hdr_t));

    ret = ipv4_route_get_hw_addr(packet->iface, dest, &dest_hw_addr);
    if(ret != SUCCESS)
        return ret;

    packet->proto = np_ipv4;

    return packet->iface->driver->tx(NULL, &dest_hw_addr, packet);
}


/*
    ipv4_reply() - reply to an IPv4 packet
*/
s32 ipv4_reply(net_packet_t *packet)
{
    ipv4_hdr_t *hdr;
    ipv4_addr_t tmp;

    packet->start -= sizeof(ipv4_hdr_t);
    packet->len += sizeof(ipv4_hdr_t);
    packet->proto = np_ipv4;

    hdr = (ipv4_hdr_t *) packet->start;

    tmp = hdr->src;
    hdr->src = hdr->dest;
    hdr->dest = tmp;

    return packet->iface->driver->reply(packet);
}


/*
    ipv4_make_addr() - populate a net_address_t object with an IPv4 address.
*/
net_address_t *ipv4_make_addr(const ipv4_addr_t ip, const ipv4_port_t port, net_address_t *addr)
{
    ipv4_address_t *ipv4_addr = (ipv4_address_t *) &addr->addr;

    addr->type = na_ipv4;
    ipv4_addr->addr = ip;
    ipv4_addr->port = port;

    return addr;
}


/*
    ipv4_alloc_packet() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 ipv4_alloc_packet(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    ks32 ret = iface->driver->alloc_packet(iface, sizeof(ipv4_hdr_t) + len, packet);
    if(ret != SUCCESS)
        return ret;

    (*packet)->start += sizeof(ipv4_hdr_t);
    (*packet)->len -= sizeof(ipv4_hdr_t);
    (*packet)->proto = np_ipv4;

    return SUCCESS;
}


/*
    ipv4_get_proto() - given a net_protocol_t-style protocol, return the corresponding IPv4 protocol
    number.
*/
ipv4_protocol_t ipv4_get_proto(const net_protocol_t proto)
{
    switch(proto)
    {
        case np_tcp:
            return ipv4_proto_tcp;

        case np_udp:
            return ipv4_proto_udp;

        case np_icmp:
            return ipv4_proto_icmp;

        default:
            return 0xff;
    }
}


/*
    ipv4_print_addr() - write addr to buf in dotted-quad format.
*/
s32 ipv4_print_addr(const ipv4_addr_t *addr, char *buf, s32 len)
{
    return snprintf(buf, len, "%u.%u.%u.%u", *addr >> 24, (*addr >> 16) & 0xff, (*addr >> 8) & 0xff,
                        *addr & 0xff);
}
