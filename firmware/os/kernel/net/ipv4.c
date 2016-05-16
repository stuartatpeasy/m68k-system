/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/ipv4route.h>
#include <kernel/net/net.h>
#include <kernel/net/packet.h>
#include <kernel/net/tcp.h>
#include <kernel/net/udp.h>
#include <klibc/stdio.h>
#include <klibc/strings.h>


// s32 ipv4_reply(net_packet_t *packet);    // FIXME
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
s32 ipv4_init()
{
    return net_protocol_register_driver(np_ipv4, "IPv4", ipv4_rx, ipv4_tx, ipv4_addr_compare);
}


/*
    ipv4_handle_packet() - handle an incoming IPv4 packet.  Return EINVAL if the packet is invalid;
    returns ESUCCESS if the packet was successfully processed, or if the packet was ignored.
*/
s32 ipv4_rx(net_packet_t *packet)
{
    ipv4_hdr_t *hdr = (ipv4_hdr_t *) net_packet_get_start(packet);

    /*
        It's usually not necessary to verify the IPv4 header checksum on received packets, as the
        hardware will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(IPV4_VERIFY_CHECKSUM)
    if(net_cksum(hdr, (hdr->version_hdr_len & 0xf) << 2) != 0x0000)
        return SUCCESS;     /* Drop packet */
#endif

    net_packet_consume(sizeof(ipv4_hdr_t), packet);
    net_packet_set_proto(np_ipv4, packet);

    // FIXME - do protocol lookup here, instead of a switch
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

    net_packet_insert(sizeof(ipv4_hdr_t), packet);

    hdr = (ipv4_hdr_t *) net_packet_get_start(packet);
    src_addr = (ipv4_address_t *) &src->addr;
    dest_addr = (ipv4_address_t *) &dest->addr;

    hdr->version_hdr_len    = (4 << 4) | 5;     /* IPv4, header len = 5 32-bit words (=20 bytes) */
    hdr->diff_svcs          = 0;
    hdr->total_len          = net_packet_get_len(packet);
    hdr->id                 = rand();           /* FIXME - rand() almost certainly wrong for pkt id */
    hdr->flags_frag_offset  = IPV4_HDR_FLAG_DF;
    hdr->ttl                = 64;               /* Sensible default? */
    hdr->protocol           = ipv4_get_proto(net_packet_get_proto(packet));
    hdr->src                = src_addr->addr;
    hdr->dest               = dest_addr->addr;
    hdr->cksum              = 0x0000;

    hdr->cksum = net_cksum(hdr, sizeof(ipv4_hdr_t));
    net_packet_set_proto(np_ipv4, packet);

    return net_tx(src, dest, packet);
}

#if 0       // FIXME
/*
    ipv4_reply() - reply to an IPv4 packet
*/
s32 ipv4_reply(net_packet_t *packet)
{
    ipv4_hdr_t *hdr;
    ipv4_addr_t tmp;

    net_packet_encapsulate(np_ipv4, sizeof(ipv4_hdr_t), packet);

    hdr = (ipv4_hdr_t *) net_packet_get_start(packet);

    tmp = hdr->src;
    hdr->src = hdr->dest;
    hdr->dest = tmp;

    return packet->iface->driver->reply(packet);
}
#endif


/*
    ipv4_make_addr() - populate a net_address_t object with an IPv4 address.
*/
net_address_t *ipv4_make_addr(const ipv4_addr_t ip, const ipv4_port_t port, net_address_t *addr)
{
    ipv4_address_t *ipv4_addr = (ipv4_address_t *) &addr->addr;

    net_address_set_type(na_ipv4, addr);
    ipv4_addr->addr = ip;
    ipv4_addr->port = port;

    return addr;
}


/*
    ipv4_packet_alloc() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 ipv4_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                      net_packet_t **packet)
{
    ks32 ret = net_packet_alloc(net_address_get_hwproto(addr), addr, sizeof(ipv4_hdr_t) + len,
                                iface, packet);
    if(ret != SUCCESS)
        return ret;

    net_packet_consume(sizeof(ipv4_hdr_t), *packet);
    net_packet_set_proto(np_ipv4, *packet);

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
    ipv4_get_addr() - if the supplied net_address_t object represents an IPv4 address, return a ptr
    to the IP address part of the address/port combination; otherwise, return NULL.
*/
const ipv4_addr_t *ipv4_get_addr(const net_address_t * const addr)
{
    if(net_address_get_type(addr) != na_ipv4)
        return NULL;

    return &((ipv4_address_t *) net_address_get_address(addr))->addr;
}


/*
    ipv4_addr_compare() - compare two IPv4 addresses.
*/
s32 ipv4_addr_compare(const net_address_t * const a1, const net_address_t * const a2)
{
    if((net_address_get_type(a1) != na_ipv4) || (net_address_get_type(a2) != na_ipv4))
        return -1;      /* Mismatch */

    return memcmp(net_address_get_address(a1), net_address_get_address(a2), sizeof(ipv4_address_t));
}


/*
    ipv4_print_addr() - write addr to buf in dotted-quad format.
*/
s32 ipv4_print_addr(const net_address_t *addr, char *buf, s32 len)
{
    const ipv4_addr_t * const a = (const ipv4_addr_t *) net_address_get_address(addr);

    return snprintf(buf, len, "%u.%u.%u.%u", *a >> 24, (*a >> 16) & 0xff, (*a >> 8) & 0xff,
                        *a & 0xff);
}
