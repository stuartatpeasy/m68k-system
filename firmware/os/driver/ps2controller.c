/*
    ATmega8-driven dual PS/2 port controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, December 2015.

    This device has driver ID 0x82.
*/

#include <driver/ps2controller.h>
#include <kernel/cpu.h>
#include <klibc/stdio.h>			/* TODO remove */


s32 ps2controller_port_a_init(dev_t *dev);
s32 ps2controller_port_b_init(dev_t *dev);
void ps2controller_irq_handler(ku32 irql, void *data);
s32 ps2controller_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
s32 ps2controller_read(dev_t *dev, ku32 offset, u32 *len, void *buf);
s32 ps2controller_write(dev_t *dev, ku32 offset, u32 *len, const void *buf);
s32 ps2controller_shut_down(dev_t *dev);


/*
    ps2controller_irq_handler() - interrupt service routine
*/
void ps2controller_irq_handler(ku32 irql, void *data)
{
    dev_t * const dev = (dev_t *) data;
    void * const base_addr = dev->base_addr;
    u8 status_a, status_b;
    UNUSED(irql);
    UNUSED(data);

    status_a = PS2CTRLR_REG(base_addr, PS2_STATUS_A);
    status_b = PS2CTRLR_REG(base_addr, PS2_STATUS_B);

    if(status_a)
    {
        if(status_a & PS2_FLAG_RX)
        {
            /* Handle received data */
        }

        /* Clear all channel A interrupt flags */
        PS2CTRLR_REG(base_addr, PS2_STATUS_A) = 0;
    }

    if(status_b)
    {

        /* Clear all channel B interrupt flags */
        PS2CTRLR_REG(base_addr, PS2_STATUS_B) = 0;
    }

    putchar('#');
}


/*
    ps2controller_init() - initialise the PS/2 ports.
*/
s32 ps2controller_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    dev_t *port_a, *port_b;
    s32 ret;

    dev->shut_down  = ps2controller_shut_down;
    dev->read       = ps2controller_read;
    dev->write      = ps2controller_write;
    dev->control    = ps2controller_control;

    dev->data = NULL;

    /* Set up child devices */
    ret = dev_create(DEV_TYPE_CHARACTER, DEV_SUBTYPE_PS2PORT, dev->name, IRQL_NONE, dev->base_addr,
                        &port_a, "PS/2 port A", dev, ps2controller_port_a_init);
    if(ret != SUCCESS)
        return ret;

    ret = dev_create(DEV_TYPE_CHARACTER, DEV_SUBTYPE_PS2PORT, dev->name, IRQL_NONE, dev->base_addr,
                        &port_b, "PS/2 port B", dev, ps2controller_port_b_init);
    if(ret != SUCCESS)
    {
        dev_destroy(port_a);
        return ret;
    }

    /* Switch port power on */
    PS2CTRLR_REG(base_addr, PS2_CFG) = PS2_CFG_PWR_A | PS2_CFG_PWR_B;

    /* Note: interrupts are disabled during initialisation.  We therefore poll for peripherals. */

    cpu_irq_add_handler(dev->irql, dev, ps2controller_irq_handler);

    /* Enable interrupts */
    PS2CTRLR_REG(base_addr, PS2_INT_CFG_A) = PS2_FLAG_RX | PS2_FLAG_PAR_ERR | PS2_FLAG_OVF;
    PS2CTRLR_REG(base_addr, PS2_INT_CFG_B) = PS2_FLAG_RX | PS2_FLAG_PAR_ERR | PS2_FLAG_OVF;

    PS2CTRLR_REG(base_addr, PS2_CFG) |= PS2_CFG_IE;

    return SUCCESS;
}


/*
    ps2controller_port_a_init() - initialise port A of the PS/2 controller.
*/
s32 ps2controller_port_a_init(dev_t *dev)
{
    UNUSED(dev);
    return SUCCESS;
}


/*
    ps2controller_port_b_init() - initialise port A of the PS/2 controller.
*/
s32 ps2controller_port_b_init(dev_t *dev)
{
    UNUSED(dev);
    return SUCCESS;
}


/*
    ps2controller_shut_down() - prepare for deletion of the device.
*/
s32 ps2controller_shut_down(dev_t *dev)
{
    void * const base_addr = dev->base_addr;

    /* Disable interrupts and deactivate power to both ports */
    PS2CTRLR_REG(base_addr, PS2_CFG) &= ~(PS2_CFG_PWR_A | PS2_CFG_PWR_B | PS2_CFG_IE);

    if(dev->data != NULL)
        kfree(dev->data);

    return SUCCESS;
}


/*
    ps2controller_port_a_getc() - read a character from PS/2 port A
*/


/*
    ps2controller_port_b_getc() - read a character from PS/2 port B
*/


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
