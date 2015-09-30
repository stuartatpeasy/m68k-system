/*
    Microchip ENCx24J600 Ethernet controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    This device has driver ID 0x81.
*/

#include "device/encx24j600.h"


const dev_driver_t g_encx24j600_device =
{
    .name       = "eth",
    .version    = 0x00000100,

    .init       = encx24j600_init,
    .shut_down  = encx24j600_shut_down,
    .read       = encx24j600_read,
    .write      = encx24j600_write,
    .control    = encx24j600_control
};

s32 encx24j600_reset(expansion_root_t *root)
{
    /* base should point to the base address of the peripheral */
    u32 x;
    ku16 init_val = 0x1234;

    /* Write init_val to EUDAST, then read the register back, until the reg contains init_val */
    x = 1000;
    do
    {
        ENCX24_REG(root->base, EUDAST) = init_val;
    } while(--x && ENCX24_REG(root->base, EUDAST) != init_val);

    if(!x)
        return ETIME;

    /* Wait for CLKRDY (ESTAT<12>) to become set */
    for(x = 10000; !(ENCX24_REG(root->base, ESTAT) & BIT(ESTAT_CLKRDY)) && --x;)
        ;

    if(!x)
        return ETIME;

    /* Issue a "system reset" command by setting ETHRST (ECON2<4>) */
    ENCX24_REG(root->base, ECON2) |= BIT(ECON2_ETHRST);

    /* Wait for the reset to complete */
    for(x = 10000; (ENCX24_REG(root->base, EUDAST) != 0x0000) && --x;)
        ;

    /* Check that EUDAST has returned to its post-reset value of 0x0000 */
    if(!x)
        return EDEVINITFAILED;

    /* Wait at least 256us for the PHY to initialise */
    for(x = 1000; --x;)
        cpu_nop();                                  /* FIXME - hardwired delay */

    return SUCCESS;
}


void encx24j600_irq(u16 irql, void *data, const struct regs *regs)
{
    expansion_root_t *root = (expansion_root_t *) data;

    /* Disable interrupts on the ENCX24 while we process this one */
//    ENCX24_REG(root->base, )
    putchar('*');
}


s32 encx24j600_init(expansion_root_t *root)
{
    s32 ret;

    /* Reset the controller */
    ret = encx24j600_reset(root);
    if(ret != SUCCESS)
        return ret;

    cpu_set_interrupt_handler(root->irql, root, encx24j600_irq);  /* Install IRQ handler */

    ENCX24_REG(root->base, ECON2) &= ~ECON2_COCON_MASK;     /* Disable the ENC's output clock */
    ENCX24_REG(root->base, ERXST) = N2LE16(0x1000);         /* Initialise packet RX buffer */

    /* Initialise receive filters */
    ENCX24_REG(root->base, ERXFCON) =
        BIT(ERXFCON_CRCEN) |                /* Reject packets with CRC errors       */
        BIT(ERXFCON_RUNTEN) |               /* Reject runt packets                  */
        BIT(ERXFCON_UCEN) |                 /* Accept unicast packets sent to us    */
        BIT(ERXFCON_BCEN);                  /* Accept broadcast packets             */

    /* Initialise MAC */
    ENCX24_REG(root->base, MACON2) =
        BIT(MACON2_DEFER) |                 /* Defer TX until medium is available           */
        (5 << MACON2_PADCFG_SHIFT) |        /* Auto-pad, understand VLAN frames, add CRC    */
        BIT(MACON2_TXCRCEN);                /* Enable auto CRC generation and append        */
        /* TODO: enable full duplex? */

    ENCX24_REG(root->base, MAMXFL) = N2LE16(1522);  /* Set max. frame length */

    /* TODO - Initialise PHY */

    /* Configure MAC inter-packet gap (MAIPG = 0x12) */
    ENCX24_REG(root->base, MAIPG) = N2LE16(0x0c12); /* high byte reserved; must be set to 0x0c */

    /* Configure LEDs: LED A - link state; LED B - TX/RX events */
    ENCX24_REG(root->base, EIDLED) = (EIDLED_L << EIDLED_LACFG_SHIFT) |
                                     (EIDLED_TR << EIDLED_LBCFG_SHIFT);

    /* Enable interrupts on link state changed and packet received events */
//    ENCX24_REG(root->base, EIE) = BIT(EIE_INTIE) | BIT(EIE_LINKIE) | BIT(EIE_PKTIE);

    ENCX24_REG(root->base, ECON1) |= BIT(ECON1_RXEN);       /* Enable packet reception */

    return SUCCESS;
}


s32 encx24j600_shut_down(expansion_root_t *root)
{
    return SUCCESS;
}


s32 encx24j600_read(expansion_root_t *root)
{
    return SUCCESS;
}


s32 encx24j600_write(expansion_root_t *root)
{
    return SUCCESS;
}


s32 encx24j600_control(expansion_root_t *root)
{
    return SUCCESS;
}
