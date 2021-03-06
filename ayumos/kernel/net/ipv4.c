/*
    IPv4 implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING
#ifdef WITH_NET_IPV4

#include <kernel/include/memory/slab.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/arp.h>
#include <kernel/include/net/net.h>
#include <kernel/include/net/packet.h>
#include <kernel/include/preempt.h>
#include <kernel/include/process.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/strings.h>


ipv4_protocol_t ipv4_get_ipproto(const net_protocol_t proto);
net_protocol_t ipv4_get_proto(const ipv4_protocol_t proto);


/* The IPv4 routing table */
ipv4_rt_item_t *g_ipv4_routes = NULL;

/* A pointer to the default route entry */
ipv4_route_t *ipv4_default_route = NULL;

static ipv4_port_alloc_bitmap_t g_ports = NULL;


/*
    ipv4_init() - initialise the IPv4 protocol driver.
*/
s32 ipv4_init()
{
    net_proto_fns_t fns;
    s32 ret;

    net_proto_fns_struct_init(&fns);

    ret = ipv4_port_alloc_bitmap_init(&g_ports);
    if(ret != SUCCESS)
        return ret;

    fns.rx              = ipv4_rx;
    fns.tx              = ipv4_tx;
    fns.addr_compare    = ipv4_addr_compare;
    fns.packet_alloc    = ipv4_packet_alloc;
    fns.route_get_iface = ipv4_route_get_iface;

    return net_protocol_register_driver(np_ipv4, "IPv4", &fns);
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
        return -ECKSUM;     /* Drop packet */
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

    iface = net_packet_get_interface(packet);
    if(!iface)
    {
        ret = ipv4_route_get_iface(dest, &iface);
        if(ret != SUCCESS)
            return ret;
    }

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
    ipv4_addr_compare() - compare two IPv4 addresses.  Return 0 for a match, nonzero otherwise.
*/
s32 ipv4_addr_compare(const net_address_t * const a1, const net_address_t * const a2)
{
    const ipv4_address_t *ip1, *ip2;

    if((net_address_get_type(a1) != na_ipv4) || (net_address_get_type(a2) != na_ipv4))
        return -1;      /* Mismatch */

    ip1 = (ipv4_address_t *) net_address_get_address(a1);
    ip2 = (ipv4_address_t *) net_address_get_address(a2);

    return !(ip1->addr == ip2->addr);
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


/*
    ipv4_route_add() - add an item to the IPv4 routing table.  Fail if the new item is a duplicate
    of an existing item.

    FIXME - ipv4_route_add should take route args individually, probably
*/
s32 ipv4_route_add(const ipv4_route_t * const r)
{
    ipv4_rt_item_t **p;
    ipv4_route_t *newent;

    if(!ipv4_mask_valid(r->mask) || (r->metric < 0))
        return -EINVAL;

    for(p = &g_ipv4_routes; *p != NULL; p = &(*p)->next)
    {
        const ipv4_route_t * const rt = &((*p)->r);

        /*
            A duplicate is either: a route with a matching destination, gateway and mask, or a route
            with destination and mask equal to 0.0.0.0 where another such route (regardless of
            gateway) already exists.
        */
        if((rt->mask == r->mask) && (rt->dest == r->dest) &&
           ((rt->gateway == r->gateway) || (rt->dest == IPV4_ADDR_NONE)))
            return -EEXIST;
    }

    *p = slab_alloc(sizeof(ipv4_rt_item_t));
    if(!*p)
        return -ENOMEM;

    newent = &((*p)->r);

    memcpy(newent, r, sizeof(ipv4_route_t));

    newent->prefix_len = ipv4_mask_to_prefix_len(r->mask);

    (*p)->next = NULL;

    /* If the new entry is the default route, point ipv4_default_route at it */
    if((r->gateway == IPV4_ADDR_NONE) && (r->mask == IPV4_MASK_NONE))
        ipv4_default_route = newent;

    return SUCCESS;
}


/*
    ipv4_route_delete() - delete an item from the IPv4 routing table.  Fail if no matching entry
    exists.
*/
s32 ipv4_route_delete(const ipv4_route_t * const r)
{
    ipv4_rt_item_t *p, **prev;

    /* Walk to the end of the routing table; fail if a duplicate entry exists */
    for(prev = NULL, p = g_ipv4_routes; p; prev = &p, p = p->next)
    {
        const ipv4_route_t * const rt = &(p->r);

        if((rt->dest == r->dest) && (rt->mask == r->mask) && (rt->gateway == r->gateway))
        {
            /* If the default route is being deleted, unset ipv4_default_route */
            if((r->gateway == IPV4_ADDR_NONE) && (r->mask == IPV4_ADDR_NONE))
                ipv4_default_route = NULL;

            if(*prev)
                (*prev)->next = p->next;
            else
                *prev = p->next;

            slab_free(p);

            return SUCCESS;
        }
    }

    return -ENOENT;
}


/*
    ipv4_route_get_entry() - can be used to iterate over routing table entries
*/
s32 ipv4_route_get_entry(ipv4_rt_item_t **e)
{
    *e = (*e == NULL) ? g_ipv4_routes : (*e)->next;

    return *e ? SUCCESS : -ENOENT;
}


/*
    ipv4_route_get() - get a routing table entry corresponding to an address.
*/
const ipv4_route_t *ipv4_route_get(const net_address_t * const proto_addr)
{
    ipv4_addr_t ipv4_addr;
    ipv4_rt_item_t *p;
    s16 best_prefix_len, best_metric;
    const ipv4_route_t *best_route;

    if(net_address_get_type(proto_addr) != na_ipv4)
        return NULL;

    ipv4_addr = ipv4_get_addr(proto_addr);

    best_prefix_len = -1;
    best_metric = -1;
    best_route = ipv4_default_route;

    for(p = g_ipv4_routes; p; p = p->next)
    {
        const ipv4_route_t * const r = &p->r;

        if((r->dest & r->mask) == (ipv4_addr & r->mask) &&
           (r->flags & IPV4_ROUTE_UP) &&
           (r->prefix_len > best_prefix_len) &&
           (r->metric > best_metric))
            {
                best_prefix_len = r->prefix_len;
                best_metric = r->metric;
                best_route = r;
            }
    }

    return best_route;
}


/*
    ipv4_route_get_iface() - get the interface associated with a particular address.  Return NULL if
    no suitable route was found and no default route exists.
*/
s32 ipv4_route_get_iface(const net_address_t * const proto_addr, net_iface_t **iface)
{
    const ipv4_route_t * const route = ipv4_route_get(proto_addr);

    if(route)
    {
        *iface = route->iface;
        return SUCCESS;
    }

    return -EHOSTUNREACH;
}


/*
    ipv4_route_get_hw_addr() - get the hardware (currently Ethernet) address corresponding to the
    specified protocol address, performing a lookup (and sleeping) if necessary.
*/
s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr)
{
    arp_cache_item_t *item;
    net_address_t ipv4_addr_broadcast;

    if(net_address_get_type(proto_addr) != na_ipv4)
        return -EHOSTUNREACH;       /* Should this be EPROTONOSUPPORT? */

    /* Check ARP cache */
    item = arp_cache_lookup(iface, proto_addr);
    if(item)
    {
        *hw_addr = item->hw_addr;
        return SUCCESS;
    }

    /* Is this a broadcast address? */
    if(!ipv4_addr_compare(proto_addr, ipv4_make_broadcast_addr(&ipv4_addr_broadcast)))
    {
        /* FIXME - this is Ethernet-specific! */
        eth_make_broadcast_addr(hw_addr);
        return SUCCESS;
    }

    /* Attempt to do ARP lookup */
    puts("Can't route yet");

    return -EHOSTUNREACH;
}


/*
    ipv4_port_alloc_bitmap_init() - initialise a port-allocation bitmap.
*/
s32 ipv4_port_alloc_bitmap_init(ipv4_port_alloc_bitmap_t *bitmap)
{
    const u16 slab_ptrs = 65536 / IPV4_PORTS_PER_SLAB;
    ipv4_port_alloc_bitmap_t bitmap_;

    bitmap_= slab_alloc(slab_ptrs * sizeof(u8 **));
    if(bitmap_ == NULL)
        return -ENOMEM;

    bzero(bitmap_, slab_ptrs * sizeof(u8 **));
    *bitmap = bitmap_;

    return SUCCESS;
}


/*
    ipv4_port_alloc_bitmap_free() - release the memory associated with a port-allocation bitmap.
*/
s32 ipv4_port_alloc_bitmap_free(ipv4_port_alloc_bitmap_t bitmap)
{
    const u16 slab_ptrs = 65536 / IPV4_PORTS_PER_SLAB;
    u16 u;

    for(u = 0; u < slab_ptrs; ++u)
        if(bitmap[u] != NULL)
        {
            u16 v;
            for(v = 0; v < (SLAB_MAX_SIZE / sizeof(u8 *)); ++v)
                if(bitmap[u][v] != NULL)
                    slab_free(bitmap[u][v]);

            slab_free(bitmap[u]);
        }

    slab_free(bitmap);

    return SUCCESS;
}


/*
    ipv4_port_alloc() - mark an available port number as allocated, and return it through
    <*port>.  The allocated/free status of port numbers is maintained as a doubly-indirect bitmap.

    Terminology:
        slab_nr     index of the slab containing pointers to the appropriate bitmaps
        bitmap_nr   index of the bitmap containing free/used status
        byte_nr     index of the bitmap byte containing free/used status
        bit_nr      the bit number, within a bitmap byte, relating to a requested port
*/
s32 ipv4_port_alloc(ipv4_port_alloc_bitmap_t alloc_bitmap, ipv4_port_t *port, ku16 type)
{
    u16 search_port, slab_nr, bitmap_nr, byte_nr;
    u8 bit_nr;

    if(type == IPV4_PORT_NUM_SPECIFIC)
    {
        /* Caller has requested a specific port number.  Set up slab, bitmap, byte and bit_offset */
        search_port = *port;

        /* If a privileged port was requested, ensure that the current task is running as uid 0 */
        if((search_port <= IPV4_PRIV_PORT_END) && proc_current_uid())
            return -EPERM;
    }
    else if(type == IPV4_PORT_NUM_EPHEMERAL)
    {
        /* Caller has requested an ephemeral port number. */
        search_port = IPV4_EPHEM_PORT_START;
    }
    else
        return -EINVAL;

    slab_nr = search_port / IPV4_PORTS_PER_SLAB;
    bitmap_nr = (search_port / IPV4_PORTS_PER_BITMAP)
                - (slab_nr * (search_port / IPV4_PORTS_PER_SLAB));
    byte_nr = (search_port / 8) - ((slab_nr * (search_port / IPV4_PORTS_PER_SLAB))
                                   + (bitmap_nr * (search_port / IPV4_PORTS_PER_BITMAP)));
    bit_nr = search_port % 8;

    preempt_disable();                                              /* BEGIN locked section */

    for(; slab_nr < (SLAB_MAX_SIZE / sizeof(u8 **)); ++slab_nr)
    {
        u8 **slab = alloc_bitmap[slab_nr];

        if(slab == NULL)
        {
            slab = slab_alloc(SLAB_MAX_SIZE);
            if(slab == NULL)
                return -ENOMEM;

            bzero(slab, SLAB_MAX_SIZE);
            alloc_bitmap[slab_nr] = slab;
        }

        for(; bitmap_nr < (SLAB_MAX_SIZE / sizeof(u8 *)); ++bitmap_nr)
        {
            u8 *bitmap = slab[bitmap_nr];

            if(bitmap == NULL)
            {
                bitmap = slab_alloc(SLAB_MAX_SIZE);
                if(bitmap == NULL)
                    return -ENOMEM;

                bzero(bitmap, SLAB_MAX_SIZE);
                slab[bitmap_nr] = bitmap;
            }

            /*
                If we're looking for a specific port: check that it's free, fail if not; mark it as
                in use and return accordingly.
            */
            if(type == IPV4_PORT_NUM_SPECIFIC)
            {
                if(bitmap[byte_nr] & (1 << bit_nr))
                {
                    preempt_enable();
                    return -EADDRINUSE;
                }

                bitmap[byte_nr] |= 1 << bit_nr;
                preempt_enable();

                return SUCCESS;
            }
            else
            {
                u8 x = (1 << bit_nr);

                for(; (bitmap[byte_nr] == 0xff) && (byte_nr < SLAB_MAX_SIZE);
                    ++byte_nr, search_port += 8)
                    ;

                if(byte_nr == SLAB_MAX_SIZE)
                    continue;

                for(; bit_nr < 8; x <<= 1, ++bit_nr)
                {
                    if(!(bitmap[byte_nr] & x))
                    {
                        if((search_port + bit_nr) >= IPV4_EPHEM_PORT_END)
                        {
                            preempt_enable();
                            return -ENFILE;     /* No ports available */
                        }

                        bitmap[byte_nr] |= x;
                        preempt_enable();

                        *port = search_port + bit_nr;

                        return SUCCESS;
                    }
                }

                search_port += 8;
            }
        }
    }

    preempt_enable();                                               /* END locked section */

    return -ENFILE;     /* No ports available */
}


/*
    ipv4_port_free() - mark an allocated port number as free.
*/
s32 ipv4_port_free(ipv4_port_alloc_bitmap_t alloc_bitmap, const ipv4_port_t port)
{
    u16 slab_nr, bitmap_nr, byte_nr;
    u8 *byte, bit_nr;

    slab_nr = port / IPV4_PORTS_PER_SLAB;
    bitmap_nr = (port / IPV4_PORTS_PER_BITMAP) - (slab_nr * (port / IPV4_PORTS_PER_SLAB));
    byte_nr = (port / 8) - ((slab_nr * (port / IPV4_PORTS_PER_SLAB))
                            + (bitmap_nr * (port / IPV4_PORTS_PER_BITMAP)));
    bit_nr = port % 8;

    byte = alloc_bitmap[slab_nr][bitmap_nr] + byte_nr;

    preempt_disable();

    if(*byte & (1 << bit_nr))
    {
        /* Port was not in use */
        *byte &= ~(1 << bit_nr);

        preempt_enable();
        return SUCCESS;
    }

    /* Port was not in use */
    preempt_enable();
    return -ENOENT;
}

#endif /* WITH_NET_IPV4 */
#endif /* WITH_NETWORKING */
