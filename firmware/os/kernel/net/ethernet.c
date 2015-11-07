/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <klibc/stdio.h>            /* FIXME remove */


/* TODO: eth_print_mac() is special-case/useless.  do this differently */
void eth_print_mac(const mac_addr_t * const mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac->b[0], mac->b[1], mac->b[2], mac->b[3], mac->b[4], mac->b[5]);
}


/*
    eth_handle_frame() - handle a received Ethernet frame
*/
void eth_handle_frame(void *frame, u32 len)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) frame;
    const void * const payload = ((u8 *) frame) + sizeof(eth_hdr_t);

    const ethertype_t etype = (ethertype_t) ehdr->type;

    put("src=");
    eth_print_mac(&ehdr->src);
    put("  dst=");
    eth_print_mac(&ehdr->dest);

    if(etype == ethertype_ipv4)
    {
        ipv4_hdr_t *ihdr = (ipv4_hdr_t *) payload;

        put("  IPv4  ");
        put("src=");
        ip_print_addr(ihdr->src);
        put("  dest=");
        ip_print_addr(ihdr->dest);
        putchar('\n');
    }
    else if(etype == ethertype_arp)
    {
        puts("  ARP");
    }
}
