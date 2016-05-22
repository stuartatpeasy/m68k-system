/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>
#include <kernel/net/ipv4route.h>
#include <kernel/net/net.h>
#include <kernel/net/packet.h>
#include <klibc/stdio.h>
#include <klibc/strings.h>


ipv4_protocol_t ipv4_get_ipproto(const net_protocol_t proto);
net_protocol_t ipv4_get_proto(const ipv4_protocol_t proto);


/*
    ipv4_init() - initialise the IPv4 protocol driver.
*/
s32 ipv4_init()
{
    return net_protocol_register_driver(np_ipv4, "IPv4", ipv4_rx, ipv4_tx, ipv4_addr_compare,
                                        ipv4_packet_alloc);
}


/*
    ipv4_handle_packet() - handle an incoming IPv4 packet by decapsulating it, optionally verifying
    its header checksum, and passing it up to the next protocol handler.
*/
s32 ipv4_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    ipv4_hdr_t *hdr = (ipv4_hdr_t *) net_packet_get_start(packet);
    net_address_t ipv4_src, ipv4_dest;
    s32 ret;
    UNUSED(dest);

    /*
        It's usually not necessary to verify the IPv4 header checksum on received packets, as the
        hardware will already have verified the checksum of the whole (e.g. Ethernet) frame.
    */
#if(IPV4_VERIFY_CHECKSUM)
    if(net_cksum(hdr, (hdr->version_hdr_len & 0xf) << 2) != 0x0000)
        return ECKSUM;      /* Drop packet */
#endif

    ret = net_packet_consume(packet, sizeof(ipv4_hdr_t));
    if(ret != SUCCESS)
        return ret;

    ipv4_make_addr(hdr->src, IPV4_PORT_NONE, &ipv4_src);
    ipv4_make_addr(hdr->dest, IPV4_PORT_NONE, &ipv4_dest);

    net_packet_set_proto(packet, ipv4_get_proto(hdr->protocol));

    /* Add the packet's source hardware address and source protocol address to the ARP cache */
    if(hdr->src != IPV4_ADDR_NONE)
        arp_cache_add(net_packet_get_interface(packet), src, &ipv4_src);

    return net_protocol_rx(&ipv4_src, &ipv4_dest, packet);
}


/*
    ipv4_tx() - transmit an IPv4 packet.
*/
s32 ipv4_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    net_address_t routed_src, routed_dest;
    net_iface_t *iface;
    ipv4_hdr_t *hdr;
    s32 ret;

    iface = net_packet_get_interface(packet) ?
        net_packet_get_interface(packet) : ipv4_route_get_iface(dest);

    if(!iface)
        return EHOSTUNREACH;

    ret = net_packet_insert(packet, sizeof(ipv4_hdr_t));
    if(ret != SUCCESS)
        return ret;

    hdr = (ipv4_hdr_t *) net_packet_get_start(packet);

    hdr->version_hdr_len    = (4 << 4) | 5;     /* IPv4, header len = 5 32-bit words (=20 bytes) */
    hdr->diff_svcs          = 0;
    hdr->total_len          = net_packet_get_len(packet);
    hdr->id                 = rand();           /* FIXME - rand() almost certainly wrong for pkt id */
    hdr->flags_frag_offset  = IPV4_HDR_FLAG_DF;
    hdr->ttl                = IPV4_DEFAULT_TTL;
    hdr->protocol           = ipv4_get_ipproto(net_packet_get_proto(packet));
    hdr->src                = ipv4_get_addr(src);
    hdr->dest               = ipv4_get_addr(dest);
    hdr->cksum              = 0x0000;

    hdr->cksum = net_cksum(hdr, sizeof(ipv4_hdr_t));

    net_packet_set_proto(packet, np_ipv4);
    net_packet_set_interface(packet, iface);

    routed_src = *net_interface_get_hw_addr(iface);

    ret = ipv4_route_get_hw_addr(iface, dest, &routed_dest);
    if(ret != SUCCESS)
        return ret;

    return net_protocol_tx(&routed_src, &routed_dest, packet);
}


/*
    ipv4_make_addr() - populate a net_address_t object with an IPv4 address and return it.
*/
net_address_t *ipv4_make_addr(const ipv4_addr_t ip, const ipv4_port_t port, net_address_t *addr)
{
    ipv4_address_t *ipv4_addr = (ipv4_address_t *) net_address_get_address(addr);

    net_address_set_type(na_ipv4, addr);
    ipv4_addr->addr = ip;
    ipv4_addr->port = port;

    return addr;
}


/*
    ipv4_addr_set_port() - set the port associated with an IPv4 address object.
*/
ipv4_address_t *ipv4_addr_set_port(ipv4_address_t * const addr, const ipv4_port_t port)
{
    addr->port = port;
    return addr;
}


/*
    ipv4_make_broadcast_addr() - populate a net_address_t object with the IPv4 broadcast address
    (255.255.255.255) and return it.
*/
net_address_t *ipv4_make_broadcast_addr(net_address_t * const addr)
{
    return ipv4_make_addr(IPV4_ADDR_BROADCAST, IPV4_PORT_NONE, addr);
}


/*
    ipv4_packet_alloc() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 ipv4_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                      net_packet_t **packet)
{
    ks32 ret = net_protocol_packet_alloc(net_address_get_hw_proto(addr), addr,
                                         sizeof(ipv4_hdr_t) + len, iface, packet);
    if(ret != SUCCESS)
        return ret;

    net_packet_set_proto(*packet, np_ipv4);

    return net_packet_consume(*packet, sizeof(ipv4_hdr_t));
}


/*
    ipv4_get_proto() - given a net_protocol_t-style protocol, return the corresponding IPv4 protocol
    number.
*/
ipv4_protocol_t ipv4_get_ipproto(const net_protocol_t proto)
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
            return ipv4_proto_invalid;
    }
}


/*
    ipv4_get_proto() - given an IPv4 protocol value, return the corresponding np_* protocol
    constant.
*/
net_protocol_t ipv4_get_proto(const ipv4_protocol_t proto)
{
    switch(proto)
    {
        case ipv4_proto_tcp:
            return np_tcp;

        case ipv4_proto_udp:
            return np_udp;

        case ipv4_proto_icmp:
            return np_icmp;

        default:
            return np_unknown;
    }
}


/*
    ipv4_get_addr() - if the supplied net_address_t object represents an IPv4 address, return a ptr
    to the IP address part of the address/port combination; otherwise, return IPV4_ADDR_NONE
    (=0.0.0.0).
*/
ipv4_addr_t ipv4_get_addr(const net_address_t * const addr)
{
    if(net_address_get_type(addr) != na_ipv4)
        return IPV4_ADDR_NONE;

    return ((ipv4_address_t *) net_address_get_address(addr))->addr;
}


/*
    ipv4_get_port() - if the supplied net_address_t object represents an IPv4 address/port object,
    return the port number associated with the object; otherwise, return IPV4_PORT_NONE (0).
*/
ipv4_port_t ipv4_get_port(const net_address_t * const addr)
{
    if(net_address_get_type(addr) != na_ipv4)
        return IPV4_PORT_NONE;

    return ((ipv4_address_t *) net_address_get_address(addr))->port;
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
    const ipv4_addr_t a = ipv4_get_addr(addr);

    return snprintf(buf, len, "%u.%u.%u.%u", a >> 24, (a >> 16) & 0xff, (a >> 8) & 0xff,
                        a & 0xff);
}


/*
    ipv4_mask_valid() - return non-zero if the specified ipv4_addr_t object represents a valid
    netmask (e.g. 0.0.0.0, 255.255.252.0, etc.)
*/
u32 ipv4_mask_valid(const ipv4_addr_t mask)
{
    return !mask || !(~mask & (~mask + 1));
}


/*
    ipv4_mask_to_prefix_len() - convert a netmask, contained in an ipv4_addr_t object, to a CIDR-
    style prefix length value.  E.g. "255.255.252.0" -> 22.
*/
u8 ipv4_mask_to_prefix_len(const ipv4_addr_t mask)
{
    u32 m;
    u8 cidr;

    for(m = ~mask, cidr = 32; m; m >>= 1, --cidr)
        ;

    return cidr;
}
