/*
    Microchip ENCx24J600 Ethernet controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    This device has driver ID 0x81.
*/

#ifdef WITH_DRV_NET_ENCX24J600
#ifndef WITH_NETWORKING
#error This driver requires kernel networking support (build option WITH_NETWORKING)
#else

#include <driver/encx24j600.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/net/ethernet.h>
#include <kernel/include/net/net.h>
#include <kernel/include/process.h>
#include <klibc/include/string.h>


s32 encx24j600_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
s32 encx24j600_packet_read(dev_t *dev, void *buf, u32 *len);
s32 encx24j600_read(dev_t *dev, ku32 offset, u32 *len, void *buf);
s32 encx24j600_reset(dev_t *dev);
void encx24j600_rx_buf_peek(dev_t *dev, u16 len, void *out);
void encx24j600_rx_buf_advance_ptr(dev_t *dev, u16 len);
s32 encx24j600_shut_down(dev_t *dev);
s32 encx24j600_write(dev_t *dev, ku32 offset, u32 *len, const void *buf);


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
    encx24j600_packet_tx() - transmit a packet.  buf points to a buffer containing the packet.
    Note that the controller will automatically prepend the source MAC and append a CRC - this data
    should not be present in buf.
*/
s32 encx24j600_packet_tx(dev_t *dev, void *buf, u32 len)
{
    void * const base_addr = dev->base_addr;

    if((len < ENCX24_MIN_TX_PACKET_LEN) || (len > ENCX24_MAX_TX_PACKET_LEN))
        return EINVAL;

    /* Clear EIR.TXIF (TX done) and EIR.TXABITIF (TX aborted) flags in interrupt flag register */
    ENCX24_REG(base_addr, EIR) &= ~(BIT(EIR_TXIF) | BIT(EIR_TXABTIF));

    /* Enable transmit done interrupt */
    ENCX24_REG(base_addr, EIE) |= BIT(EIE_TXIE);

    /* Copy the packet into the transmit buffer */
    memcpy((u8 *) base_addr + ENCX24_TX_BUF_START, buf, len);

    /* Set ETXST to the start address of the packet in the transmit buffer */
    ENCX24_REG(base_addr, ETXST) = N2LE16(ENCX24_TX_BUF_START);

    /* Set ETXLEN to the length of the packet */
    ENCX24_REG(base_addr, ETXLEN) = N2LE16(len);

    /* Set TXRTS to begin transmission */
    ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_TXRTS);

    return SUCCESS;
}


/*
    encx24j600_rx_buf_peek() - read len bytes from the RX buffer into out, without advancing the
    RX buffer read pointer.
*/
void encx24j600_rx_buf_peek(dev_t *dev, u16 len, void *out)
{
    void * const base_addr = (void *) dev->base_addr;
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    ku16 *rx_buf_top = (u16 *) base_addr + (ENCX24_MEM_TOP / sizeof(u16));
    u16 *out16 = (u16 *) out,
        *rx_read_ptr = state->rx_read_ptr;
    u16 curr_part_len;

    len = (len + 1) / sizeof(u16);

    curr_part_len = MIN(len, (u16) (rx_buf_top - rx_read_ptr));

    for(len -= curr_part_len; curr_part_len--;)
        *out16++ = *rx_read_ptr++;

    if(len)
    {
        rx_read_ptr = state->rx_buf_start;

        while(len--)
            *out16++ = *rx_read_ptr++;
    }
}


/*
    encx24j600_rx_buf_advance_ptr() - advance the RX buffer read pointer by len bytes, and update
    the RX tail pointer (ERXTAIL register) accordingly.
*/
void encx24j600_rx_buf_advance_ptr(dev_t *dev, u16 len)
{
    void * const base_addr = (void *) dev->base_addr;
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    ku16 *rx_buf_top = (u16 *) base_addr + (ENCX24_MEM_TOP / sizeof(u16));

    state->rx_read_ptr += (len + 1) / sizeof(u16);

    while(state->rx_read_ptr > rx_buf_top)
        state->rx_read_ptr -= ENCX24_RX_BUF_LEN_WORDS;

    ENCX24_REG(base_addr, ERXTAIL) =
        (state->rx_read_ptr == state->rx_buf_start) ?
            N2LE16(ENCX24_MEM_TOP - 2) :
            N2LE16((state->rx_read_ptr - state->rx_buf_start - 1) << 1);
}


/*
    encx24j600_packet_read() - read a received frame from the controller's buffer.
*/
s32 encx24j600_packet_read(dev_t *dev, void *buf, u32 *len)
{
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    void * const base_addr = dev->base_addr;
    encx24j600_rxhdr_t hdr;

    encx24j600_rx_buf_peek(dev, sizeof(hdr), &hdr);

    if(hdr.rsv.status & BIT(ENCX24_RSV_STAT_OK))
    {
        /* Packet received successfully */
        ku16 packet_len = N2LE16(hdr.rsv.byte_count_le) - sizeof(eth_cksum_t);

        if(*len < packet_len)
        {
            *len = packet_len;
            return EFBIG;   /* Packet too big for the supplied buffer */
        }

        *len = packet_len;

        encx24j600_rx_buf_advance_ptr(dev, sizeof(hdr));

        encx24j600_rx_buf_peek(dev, packet_len, buf);
        encx24j600_rx_buf_advance_ptr(dev, packet_len + sizeof(eth_cksum_t));

        state->rx_read_ptr = (u16 *) ((u32) base_addr + N2LE16(hdr.next_packet_ptr));
        ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_PKTDEC);
    }
    else
    {
        /* Something wrong with the packet - discard it */
        if(hdr.next_packet_ptr == ((u32) state->rx_buf_start - (u32) base_addr))
            ENCX24_REG(base_addr, ERXTAIL) = N2LE16(ENCX24_MEM_TOP - 2);
        else
            ENCX24_REG(base_addr, ERXTAIL) = N2LE16(hdr.next_packet_ptr - 2);

        state->rx_read_ptr = (u16 *) ((u32) base_addr + N2LE16(hdr.next_packet_ptr));
        ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_PKTDEC);

        /* Indicate packet discard by returning a success status with 0 bytes read. */
        *len = 0;
    }

    return SUCCESS;
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
        {
            /* Set MAC duplex configuration to match PHY duplex configuration */
            if(ENCX24_REG(base_addr, ESTAT) & BIT(ESTAT_PHYDPX))
                ENCX24_REG(base_addr, MACON2) |= BIT(MACON2_FULDPX);
            else
                ENCX24_REG(base_addr, MACON2) &= ~BIT(MACON2_FULDPX);

            state->flags |= BIT(ENCX24_STATE_LINKED);
        }
        else
            state->flags &= ~BIT(ENCX24_STATE_LINKED);
    }

    if(iflags & BIT(EIR_PKTIF))         /* Packet received    */
    {
        ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_PKTDEC);

        ++(state->rx_packets_pending);

        if(state->rx_wait_pid)
            proc_wake_by_id(state->rx_wait_pid);
    }

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

    state = slab_calloc(sizeof(encx24j600_state_t));
    if(state == NULL)
        return ENOMEM;

    /* Reset the controller */
    ret = encx24j600_reset(dev);
    if(ret != SUCCESS)
        return ret;

    cpu_irq_add_handler(dev->irql, dev, encx24j600_irq);  /* Install IRQ handler */

    ENCX24_REG(base_addr, ECON2) &= ~ECON2_COCON_MASK;     /* Disable the ENC's output clock */

    /* Initialise packet RX buffer */
    state->rx_buf_start = (u16 *) ((u8 *) base_addr + ENCX24_RX_BUF_START);
    state->rx_read_ptr = state->rx_buf_start;
    ENCX24_REG(base_addr, ERXST) = N2LE16(ENCX24_RX_BUF_START);

    /* Initialise packet TX buffer */
    ENCX24_REG(base_addr, ETXST) = 0;

    /* Disable automatic source MAC transmission */
    ENCX24_REG(base_addr, ECON2) &= ~BIT(ECON2_TXMAC);

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

    ENCX24_REG(base_addr, MAMXFL) = N2LE16(ENCX24_PACKET_LEN_MAX);  /* Set max. frame length */

    /* TODO - Initialise PHY */

    /* Configure MAC inter-packet gap (MAIPG = 0x12) */
    ENCX24_REG(base_addr, MAIPG) = N2LE16(0x0c12); /* high byte reserved; must be set to 0x0c */

    /* Configure LEDs: LED A - link state; LED B - TX/RX events */
    ENCX24_REG(base_addr, EIDLED) = (EIDLED_L << EIDLED_LACFG_SHIFT) |
                                     (EIDLED_TR << EIDLED_LBCFG_SHIFT);

    /* Enable interrupts on link state changed and packet received events */
    ENCX24_REG(base_addr, EIE) = BIT(EIE_INTIE) | BIT(EIE_LINKIE) | BIT(EIE_PKTIE);

    ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_RXEN);       /* Enable packet reception */

    dev->control = encx24j600_control;
    dev->read = encx24j600_read;
    dev->write = encx24j600_write;
    dev->shut_down = encx24j600_shut_down;

    dev->data = state;

    return SUCCESS;
}


/*
    encx24j600_shut_down() - shut down the ENCx24J600 controller, in preparation for deletion of the
    device.
*/
s32 encx24j600_shut_down(dev_t *dev)
{
    if(dev->data != NULL)
        slab_free(dev->data);

    return SUCCESS;
}


/*
    encx24j600_read() - block until a packet can be read into buf.
*/
s32 encx24j600_read(dev_t *dev, ku32 offset, u32 *len, void *buf)
{
    encx24j600_state_t * const state = (encx24j600_state_t *) dev->data;
    s32 ret;
    UNUSED(offset);

    if(state->rx_wait_pid)
        return EBUSY;

    /* Is a packet available? */
    while(state->rx_packets_pending == 0)
    {
        /* No packets available - put process to sleep */
        /* TODO - locking */
        state->rx_wait_pid = proc_get_pid();
        proc_sleep();
    }

    state->rx_wait_pid = 0;

    ret = encx24j600_packet_read(dev, buf, len);

    if(ret == SUCCESS)
        state->rx_packets_pending--;

    return ret;
}


/*
    encx24j600_write() - block until a packet can be written to the TX buffer.
*/
s32 encx24j600_write(dev_t *dev, ku32 offset, u32 *len, const void *buf)
{
    void * const base_addr = dev->base_addr;
    u32 len_ = *len;
    UNUSED(offset);

    if((len_ < ENCX24_PACKET_LEN_MIN) || (len_ > ENCX24_PACKET_LEN_MAX))
        return EINVAL;

    /* Is a transmission in progress? */
    /* FIXME - put process to sleep, instead of busy-waiting */
    while(ENCX24_REG(base_addr, ECON1) & BIT(ECON1_TXRTS))
        ;

    /* Copy the packet into the TX buffer */
    memcpy((void *) ENCX24_MEM_ADDR(base_addr, ENCX24_TX_BUF_START), buf, len_);
    ENCX24_REG(base_addr, ETXLEN) = N2LE16(len_);

    /* Transmit the packet */
    ENCX24_REG(base_addr, ECON1) |= BIT(ECON1_TXRTS);

    return SUCCESS;
}


/*
    encx24j600_control() - devctl responder
*/
s32 encx24j600_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out)
{
    const void * const base_addr = dev->base_addr;
    UNUSED(in);

    switch(fn)
    {
        case dc_get_hw_protocol:
            *((net_protocol_t *) out) = np_ethernet;
            return SUCCESS;

        case dc_get_hw_addr_type:
            *((net_addr_type_t *) out) = na_ethernet;
            return SUCCESS;

        case dc_get_hw_addr:
            ((mac_addr_t *) out)->w[0] = ENCX24_REG(base_addr, MAADR1);
            ((mac_addr_t *) out)->w[1] = ENCX24_REG(base_addr, MAADR2);
            ((mac_addr_t *) out)->w[2] = ENCX24_REG(base_addr, MAADR3);
            return SUCCESS;

        case dc_get_link_flags:
            *((u32 *) out) = ((encx24j600_state_t *) dev->data)->flags;
            return SUCCESS;

        default:
            return ENOSYS;
    }
}

#endif /* WITH_NETWORKING */
#endif /* WITH_DRV_NET_ENCX24J600 */
