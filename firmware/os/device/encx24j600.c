/*
    Microchip ENCx24J600 Ethernet controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    This device has driver ID 0x81.
*/

#include "device/encx24j600.h"


const device_driver_t g_encx24j600_device =
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
    for(x = 1000; !(ENCX24_REG(root->base, ESTAT) & BIT(ESTAT_CLKRDY)) && --x;)
        ;

    if(!x)
        return ETIME;

    /* Issue a "system reset" command by setting ETHRST (ECON2<4>) */
    ENCX24_REG(root->base, ECON2) |= BIT(ECON2_ETHRST);

    /* Wait for the reset to complete */
    for(x = 100; --x;)              /* FIXME - hardwired delay */
        ;

    /* Check that EUDAST has returned to its post-reset value of 0x0000 */
    if(ENCX24_REG(root->base, EUDAST) != 0x0000)
        return EDEVINITFAILED;

    /* Wait at least 256us for the PHY to initialise */
    for(x = 1000; --x;)
        ;                           /* FIXME - hardwired delay */

    return SUCCESS;
}


IRQ_HANDLER_FN(encx24j600_irq)
{

}


s32 encx24j600_init(expansion_root_t *root)
{
    s32 ret;

    /* Reset the controller */
    ret = encx24j600_reset(root);
    if(ret != SUCCESS)
        return ret;

    cpu_set_handler(root->irql, encx24j600_irq);            /* Install IRQ handler */

    ENCX24_REG(root->base, ECON2) = ~ECON2_COCON_MASK;      /* Disable the ENC's output clock */
    ENCX24_REG(root->base, ERXST) = 0x1000;                 /* Initialise packet RX buffer */

    /* TODO - Initialise receive filters */

    /* TODO - Initialise MAC */

    /* TODO - Initialise PHY */

    /* Configure MAC inter-packet gap (MAIPG = 0x12) */
    ENCX24_REG(root->base, MAIPG) = 0x0c12; /* high byte reserved bits must be set to 0x0c */

    /* Enable interrupts on link state changed and packet received events */
    ENCX24_REG(root->base, EIE) = BIT(EIE_INTIE) | BIT(EIE_LINKIE) | BIT(EIE_PKTIE);

    ENCX24_REG(root->base, ECON1SET) = BIT(ECON1_RXEN);     /* Enable packet reception */

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
