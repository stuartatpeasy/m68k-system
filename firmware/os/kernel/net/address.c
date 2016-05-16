/*
    Network address abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME: in net_print_address, look up driver by address type and use ->print_addr()
            then remove #includes for protos
*/

#include <kernel/net/address.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>


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
    if((type < na_unknown) || (type >= na_invalid))
        return EINVAL;

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
