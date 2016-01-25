/*
    ATmega8-driven dual PS/2 port controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, December 2015.

    This device has driver ID 0x82.
*/

#include <driver/ps2controller.h>
#include <klibc/stdio.h>			/* TODO remove */


s32 ps2controller_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
s32 ps2controller_read(dev_t *dev, ku32 offset, u32 *len, void *buf);
s32 ps2controller_write(dev_t *dev, ku32 offset, u32 *len, const void *buf);
s32 ps2controller_shut_down(dev_t *dev);



/*
    ps2controller_irq() - interrupt service routine
*/
void ps2controller_irq(ku32 irql, void *data)
{
    UNUSED(irql);
    UNUSED(data);
}


/*
    ps2controller_init() - initialise the PS/2 ports.
*/
s32 ps2controller_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;

    /* Switch port power on */
    PS2CTRLR_REG(base_addr, PS2_CFG) = BIT(PS2_CFG_PWR_A) | BIT(PS2_CFG_PWR_B);

    /* Note: interrupts are disabled during initialisation.  We therefore poll for peripherals. */

    return SUCCESS;
}


/*
    ps2controller_shut_down() - prepare for deletion of the device.
*/
s32 ps2controller_shut_down(dev_t *dev)
{
    if(dev->data != NULL)
        kfree(dev->data);

    return SUCCESS;
}


/*
    ps2controller_read() - read data from the PS/2 port controller.
*/
s32 ps2controller_read(dev_t *dev, ku32 offset, u32 *len, void *buf)
{
    UNUSED(dev);
    UNUSED(offset);
    UNUSED(len);
    UNUSED(buf);

    return SUCCESS;
}


/*
    ps2controller_write() - write data to the PS/2 port controller.
*/
s32 ps2controller_write(dev_t *dev, ku32 offset, u32 *len, const void *buf)
{
    UNUSED(dev);
    UNUSED(offset);
    UNUSED(len);
    UNUSED(buf);

    return SUCCESS;
}


/*
    ps2controller_control() - devctl responder
*/
s32 ps2controller_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out)
{
    const void * const base_addr = dev->base_addr;
    UNUSED(in);
    UNUSED(out);
    UNUSED(base_addr);

    switch(fn)
    {
        default:
            return ENOSYS;
    }
}
