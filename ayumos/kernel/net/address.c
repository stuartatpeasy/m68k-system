/*
    Network address abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME: in net_print_address, look up driver by address type and use ->print_addr()
            then remove #includes for protos
*/

#ifdef WITH_NETWORKING

#include <kernel/include/net/address.h>
#include <kernel/include/net/ethernet.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/route.h>


/*
    net_address_compare() - compare two network address via the appropriate driver.
*/
s32 net_address_compare(const net_address_t * const a1, const net_address_t * const a2)
{
    if(a1->type == a2->type)
        return net_protocol_addr_compare(net_protocol_from_address(a1), a1, a2);

    return -1;      /* Mismatch */
}



/*
    net_address_get_type() - return the address type of an address.
*/
net_addr_type_t net_address_get_type(const net_address_t * const addr)
{
    return addr->type;
}


/*
    net_address_set_type() - set the type of an address.
*/
s32 net_address_set_type(const net_addr_type_t type, net_address_t * const addr)
{
    if(type >= na_invalid)
        return -EINVAL;

    addr->type = type;
    return SUCCESS;
}


/*
    net_address_get_address() - return a pointer to the network address stored in a net_address_t
    object.
*/
const void *net_address_get_address(const net_address_t * const addr)
{
    return &addr->addr;
}


/*
    net_address_get_proto() - get the network protocol associated with an address.
*/
net_protocol_t net_address_get_proto(const net_address_t * const addr)
{
    switch(addr->type)
    {
        case na_ethernet:
            return np_ethernet;

        case na_ipv4:
            return np_ipv4;

        case na_ipv6:
            return np_ipv6;

        default:
            return np_unknown;
    }
}


/*
    net_address_type_from_proto() - given a net_protocol_t, return the corresponding
    net_addr_type_t.
*/
net_addr_type_t net_address_type_from_proto(const net_protocol_t proto)
{
    switch(proto)
    {
        case np_ethernet:
            return na_ethernet;

        case np_ipv4:
            return na_ipv4;

        case np_ipv6:
            return na_ipv6;

        default:
            return na_unknown;
    }
}


/*
    net_address_get_hw_proto() - perform a routing lookup on the given address, and return the
    hardware protocol associated with the route.
*/
net_protocol_t net_address_get_hw_proto(const net_address_t * const addr)
{
    net_iface_t *iface;

    return (net_route_get_iface(addr, &iface) == SUCCESS) ?
                net_interface_get_proto(iface) : np_unknown;
}


/*
    net_address_print() - print a human-readable form of addr into buf.
*/
s32 net_address_print(const net_address_t * const addr, char * const buf, s32 len)
{
    if(addr->type == na_ethernet)
        return eth_print_addr(addr, buf, len);
    else if(addr->type == na_ipv4)
        return ipv4_print_addr(addr, buf, len);
    else
        return -EINVAL;
}

#endif /* WITH_NETWORKING */
