/*
    Microchip ENCx24J600 Ethernet controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    This device has driver ID 0x81.
*/

#include <device/encx24j600.h>
#include <klibc/stdio.h>			/* TODO remove */
#include <kernel/net/ipv4.h>        /* TODO remove */


/*
    encx24j600_reset() - reset the ENCx24J600 controller IC
*/
s32 encx24j600_reset(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    u32 x;
    ku16 init_val = 0x1234;

    /* Write init_val to EUDAST, then read the register back, until the reg contains init_val */
    x = 1000;
    do
    {
        ENCX24_REG(base_addr, EUDAST) = init_val;
    } while(--x && ENCX24_REG(base_addr, EUDAST) != init_val);

    if(!x)
        return ETIME;

    /* Wait for CLKRDY (ESTAT<12>) to become set */
    for(x = 10000; !(ENCX24_REG(base_addr, ESTAT) & BIT(ESTAT_CLKRDY)) && --x;)
        ;

    if(!x)
        return ETIME;

    /* Issue a "system reset" command by setting ETHRST (ECON2<4>) */
    ENCX24_REG(base_addr, ECON2) |= BIT(ECON2_ETHRST);

    /* Wait for the reset to complete */
    for(x = 10000; (ENCX24_REG(base_addr, EUDAST) != 0x0000) && --x;)
        ;

    /* Check that EUDAST has returned to its post-reset value of 0x0000 */
    if(!x)
        return EDEVINITFAILED;

    /* Wait at least 256us for the PHY to initialise */
    for(x = 1000; --x;)
        cpu_nop();                                  /* FIXME - hardwired delay */

    return SUCCESS;
}


/*
    encx24j600_rx_buf_read() - perform a wrapping read of len bytes from the RX buffer; update
    the buffer-tail pointer register (ERXTAIL) appropriately.
*/
void encx24j600_rx_buf_read(dev_t *dev, u16 len, void *out)
{
    void * const base_addr = (void *) dev->base_addr;
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    ku16 *rx_buf_top = (u16 *) base_addr + (ENCX24_MEM_TOP / sizeof(u16));
    u16 *out16 = (u16 *) out;
    u16 curr_part_len;

    out16 = (u16 *) out;
    len >>= 1;

    curr_part_len = MIN(len, (u16) (rx_buf_top - state->rx_read_ptr));

    for(len -= curr_part_len; curr_part_len--;)
        *out16++ = *state->rx_read_ptr++;

    if(len)
    {
        state->rx_read_ptr = state->rx_buf_start;

        while(len--)
            *out16++ = *state->rx_read_ptr++;
    }

    ENCX24_REG(base_addr, ERXTAIL) =
        (state->rx_read_ptr == state->rx_buf_start) ?
            N2LE16(ENCX24_MEM_TOP - 2) :
            N2LE16((state->rx_read_ptr - state->rx_buf_start - 1) << 1);
}


/*
    encx24j600_packet_read() - read a received frame from the controller's buffer.
*/
void encx24j600_packet_read(dev_t *dev)
{
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    void * const base_addr = dev->base_addr;
    encx24j600_rxhdr_t hdr;

    encx24j600_rx_buf_read(dev, sizeof(hdr), &hdr);

    if(hdr.rsv.status & BIT(ENCX24_RSV_STAT_OK))
    {
        /* Packet received successfully */
//        printf("~R: npp=%04x zero=%02x rsv4=%02x rsv3=%02x stat=%02x bc=%u\n", N2LE16(hdr.next_packet_ptr),
//                hdr.rsv.zero, hdr.rsv.rsv4, hdr.rsv.rsv3, hdr.rsv.status, N2LE16(hdr.rsv.byte_count_le));

        eth_handle_frame(state->rx_read_ptr, LE2N16(hdr.rsv.byte_count_le));
    }
    else
    {
        /* Something wrong with the packet - discard it */
        if(hdr.next_packet_ptr == ((u32) state->rx_buf_start - (u32) base_addr))
            ENCX24_REG(base_addr, ERXTAIL) = N2LE16(ENCX24_MEM_TOP - 2);
        else
            ENCX24_REG(base_addr, ERXTAIL) = N2LE16(hdr.next_packet_ptr - 2);
    }

    state->rx_read_ptr = (u16 *) ((u32) base_addr + N2LE16(hdr.next_packet_ptr));

    ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_PKTDEC);
}


/*
    encx24j600_irq() - interrupt service routine
*/
void encx24j600_irq(ku32 irql, void *data)
{
    dev_t * const dev = (dev_t *) data;
    void * const base_addr = dev->base_addr;
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    u16 iflags;
    UNUSED(irql);

    /* Read the interrupt flag register (EIR) to find out the cause of the interrupt */
    iflags = ENCX24_REG(base_addr, EIR);

    if(iflags & BIT(EIR_LINKIF))        /* Link state changed */
    {
        if(ENCX24_REG(base_addr, ESTAT) & BIT(ESTAT_PHYLINK))
            state->flags |= BIT(ENCX24_STATE_LINKED);
        else
            state->flags &= ~BIT(ENCX24_STATE_LINKED);
    }

    if(iflags & BIT(EIR_PKTIF))         /* Packet received    */
        encx24j600_packet_read(dev);

    /* Clear all interrupts */
    ENCX24_REG(base_addr, EIR) = 0;
}


/*
    encx24j600_init() - initialise the ENCx24J600 controller IC.
*/
s32 encx24j600_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    encx24j600_state_t *state;
    s32 ret;
    ku32 rx_buf_start = 0x1000;

    dev->data = kmalloc(sizeof(encx24j600_state_t));
    if(dev->data == NULL)
        return ENOMEM;

    state = dev->data;

    /* Reset the controller */
    ret = encx24j600_reset(dev);
    if(ret != SUCCESS)
        return ret;

    cpu_set_interrupt_handler(dev->irql, dev, encx24j600_irq);  /* Install IRQ handler */

    ENCX24_REG(base_addr, ECON2) &= ~ECON2_COCON_MASK;     /* Disable the ENC's output clock */

    /* Initialise packet RX buffer */
    state->rx_buf_start = (u16 *) ((u8 *) base_addr + rx_buf_start);
    state->rx_read_ptr = state->rx_buf_start;
    ENCX24_REG(base_addr, ERXST) = N2LE16(rx_buf_start);

    /* Initialise receive filters */
    ENCX24_REG(base_addr, ERXFCON) =
        BIT(ERXFCON_CRCEN) |                /* Reject packets with CRC errors       */
        BIT(ERXFCON_RUNTEN) |               /* Reject runt packets                  */
        BIT(ERXFCON_UCEN) |                 /* Accept unicast packets sent to us    */
        BIT(ERXFCON_BCEN);                  /* Accept broadcast packets             */

    /* Initialise MAC */
    ENCX24_REG(base_addr, MACON2) =
        BIT(MACON2_DEFER) |                 /* Defer TX until medium is available           */
        (5 << MACON2_PADCFG_SHIFT) |        /* Auto-pad, understand VLAN frames, add CRC    */
        BIT(MACON2_TXCRCEN);                /* Enable auto CRC generation and append        */
        /* TODO: enable full duplex? */

    ENCX24_REG(base_addr, MAMXFL) = N2LE16(1522);  /* Set max. frame length */

    /* TODO - Initialise PHY */

    /* Configure MAC inter-packet gap (MAIPG = 0x12) */
    ENCX24_REG(base_addr, MAIPG) = N2LE16(0x0c12); /* high byte reserved; must be set to 0x0c */

    /* Configure LEDs: LED A - link state; LED B - TX/RX events */
    ENCX24_REG(base_addr, EIDLED) = (EIDLED_L << EIDLED_LACFG_SHIFT) |
                                     (EIDLED_TR << EIDLED_LBCFG_SHIFT);

    /* Enable interrupts on link state changed and packet received events */
    ENCX24_REG(base_addr, EIE) = BIT(EIE_INTIE) | BIT(EIE_LINKIE) | BIT(EIE_PKTIE);

    ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_RXEN);       /* Enable packet reception */

    return SUCCESS;
}


/*
    encx24j600_shut_down() - shut down the ENCx24J600 controller, in preparation for deletion of the
    device.
*/
s32 encx24j600_shut_down(dev_t *dev)
{
    if(dev->data != NULL)
        kfree(dev->data);

    return SUCCESS;
}


s32 encx24j600_read(dev_t *dev)
{
    UNUSED(dev);

    return SUCCESS;
}


s32 encx24j600_write(dev_t *dev)
{
    UNUSED(dev);

    return SUCCESS;
}


s32 encx24j600_control(dev_t *dev)
{
    UNUSED(dev);

    return SUCCESS;
}
