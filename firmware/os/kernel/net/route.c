/*
    Network routing abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME - remove printf()s and the #include stdio.h
*/

#include <klibc/stdio.h>
#include <kernel/net/route.h>


/*
    net_route_get() - look up a route in the kernel routing table.
*/
net_iface_t *net_route_get(const net_address_t *addr)
{
    char buf[64];

    /* FIXME */
    net_address_print(addr, buf, 64);
    printf("net_route_get(%s): failing (unimplemented)\n", buf);

    return NULL;
}
