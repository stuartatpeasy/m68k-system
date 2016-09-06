/*
    ATmega8-driven dual PS/2 port controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, December 2015.

    This device has driver ID 0x82.
*/

#include <driver/ps2controller.h>
#include <kernel/cpu.h>
#include <klibc/stdio.h>			/* TODO remove */


void ps2controller_port_irq_handler(ku32 irql, void *data);
s32 ps2controller_port_a_init(dev_t *dev);
s32 ps2controller_port_b_init(dev_t *dev);
s32 ps2controller_port_init(dev_t *dev, ku16 reg_offset);
s32 ps2controller_port_shut_down(dev_t *dev);
s32 ps2controller_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
s32 ps2controller_read(dev_t *dev, ku32 offset, u32 *len, void *buf);
s32 ps2controller_write(dev_t *dev, ku32 offset, u32 *len, const void *buf);
s32 ps2controller_shut_down(dev_t *dev);


/*
    ps2controller_init() - initialise the PS/2 ports.
*/
s32 ps2controller_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    ps2controller_state_t *state;
    s32 ret;

    state = CHECKED_KCALLOC(1, sizeof(ps2controller_state_t));

    dev->data = state;

    dev->shut_down  = ps2controller_shut_down;
    dev->read       = ps2controller_read;
    dev->write      = ps2controller_write;
    dev->control    = ps2controller_control;

    dev->data = NULL;

    /* Set up child devices */
    ret = dev_create(DEV_TYPE_CHARACTER, DEV_SUBTYPE_PS2PORT, dev->name, dev->irql, dev->base_addr,
                        &state->port_a, "PS/2 port A", dev, ps2controller_port_a_init);
    if(ret != SUCCESS)
    {
        kfree(state);
        dev->data = NULL;
        return ret;
    }

    ret = dev_create(DEV_TYPE_CHARACTER, DEV_SUBTYPE_PS2PORT, dev->name, dev->irql, dev->base_addr,
                        &state->port_b, "PS/2 port B", dev, ps2controller_port_b_init);
    if(ret != SUCCESS)
    {
        dev_destroy(state->port_a);
        kfree(state);
        dev->data = NULL;
        return ret;
    }

    /* Switch port power on */
    PS2CTRLR_REG(base_addr, PS2_CFG) = PS2_CFG_PWR_A | PS2_CFG_PWR_B;

    /* Activate the controller's global interrupt enable */
    PS2CTRLR_REG(base_addr, PS2_CFG) |= PS2_CFG_IE;

    return SUCCESS;
}


/*
    ps2controller_port_a_init() - initialise port A of the PS/2 controller.
*/
s32 ps2controller_port_a_init(dev_t *dev)
{
    return ps2controller_port_init(dev, PS2_PORT_A_REG_OFFSET);
}


/*
    ps2controller_port_b_init() - initialise port A of the PS/2 controller.
*/
s32 ps2controller_port_b_init(dev_t *dev)
{
    return ps2controller_port_init(dev, PS2_PORT_B_REG_OFFSET);
}


/*
    ps2controller_port_init() - initialise the specified port of the PS/2 controller.
*/
s32 ps2controller_port_init(dev_t *dev, ku16 reg_offset)
{
    ps2controller_port_state_t *state = CHECKED_KCALLOC(1, sizeof(ps2controller_port_state_t));

    state->err = 0;

    state->regs.data    = PS2CTRLR_REG_ADDR(dev->base_addr, reg_offset + PS2_DATA);
    state->regs.status  = PS2CTRLR_REG_ADDR(dev->base_addr, reg_offset + PS2_STATUS);
    state->regs.int_cfg = PS2CTRLR_REG_ADDR(dev->base_addr, reg_offset + PS2_INT_CFG);

    CIRCBUF_INIT(state->tx_buf);

    dev->data = state;

    cpu_irq_add_handler(dev->irql, dev, ps2controller_port_irq_handler);

    /* Enable interrupts */
    *state->regs.int_cfg = PS2_FLAG_RX | PS2_FLAG_PAR_ERR | PS2_FLAG_OVF;

    return SUCCESS;
}


/*
    ps2controller_port_irq_handler() - handle the interrupt raised by the flags in the register
    specified by reg.
*/
void ps2controller_port_irq_handler(ku32 irql, void *data)
{
    dev_t * const dev = (dev_t *) data;
    ps2controller_port_state_t *state = (ps2controller_port_state_t *) dev->data;
    UNUSED(irql);

    ku8 status = *state->regs.status;
    if(status)
    {
        if(status & PS2_FLAG_RX)        // change to while()
        {
            ku8 key = *state->regs.data;

            if(key == PS2_SC_KB_EXT1)
                state->packet.kb.flags |= PS2_PKT_KB_EXT1;
            else if(key == PS2_SC_KB_EXT2)
                state->packet.kb.flags |= PS2_PKT_KB_EXT2;
            else if(key == PS2_SC_KB_RELEASE)
                state->packet.kb.flags |= PS2_PKT_KB_RELEASE;
            else
            {
                ku8 flags = state->packet.kb.flags & (PS2_PKT_KB_EXT1 | PS2_PKT_KB_EXT2);

                state->packet.kb.data = (state->packet.kb.data << 8) | key;

                switch(flags)
                {
                    case 0:
                        /* If neither of the EXTx flags is true, this must be a single-byte code */
                        ps2controller_process_key(state);
                        break;

                    case PS2_PKT_KB_EXT1:
                        /*
                            Most EXT1-prefixed keys are single-byte codes; two aren't.  We see
                            "PrtSc pressed" as 0x127c, and "PrtSc released" as 0x7c12", with the
                            appropriate flags set.  We defer processing the key if we have received
                            only the first byte of either of these two (0x127c / 0x7c12) scan codes.
                            Otherwise, the scan code is processed now.
                        */
                        if(((flags & PS2_SC_KB_RELEASE) && (state->packet.kb.data != 0x7c)) ||
                           (!(flags & PS2_SC_KB_RELEASE) && (state->packet.kb.data != 0x12)))
                            ps2controller_process_key(state);
                        break;

                    case PS2_PKT_KB_EXT2:
                        /*
                            The only EXT2-prefixed code we recognise is "Pause pressed", which we
                            receive as 0x14771477.  The scan code is actually 0xe1, 0x14, 0x77,
                            0xe1, 0xf0, 0x14, 0xf0, 0x77.  Note that misplaced prefixes will not be
                            detected.  If we have received any four bytes (not including the prefix
                            characters 0xe1 and 0xf0), we process the key.
                        */
                        if((flags & PS2_SC_KB_RELEASE) && (state->packet.kb.data & 0xff00000))
                            ps2controller_process_key(state);
                        break;

                    case PS2_PKT_KB_EXT1 | PS2_PKT_KB_EXT2:
                        /* Invalid state - discard packet */
                        break;
                }

                state->packet.kb.data = 0;
                state->packet.kb.flags = 0;
            }
        }
        else if(status & PS2_FLAG_TX)
        {
            /* TODO: send next byte in TX buffer, if any */
            putchar('t');
        }
        else if(status & PS2_ERR_MASK)
        {
            state->err |= status & PS2_ERR_MASK;
            printf("[ps2e: %02x]", status & PS2_ERR_MASK);      // FIXME remove this
        }

        *state->regs.status = 0;
    }
}


void ps2controller_process_key(ps2controller_port_state_t *state)
{
    if(state->packet.kb.flags & PS2_PKT_KB_RELEASE)
        putchar('!');

    if(state->packet.kb.flags & PS2_PKT_KB_EXT1)
        putchar('1');

    if(state->packet.kb.flags & PS2_PKT_KB_EXT2)
        putchar('2');

    printf(":%08x ", state->packet.kb.data);
}


/*
    ps2controller_shut_down() - prepare for deletion of the controller device.  Note that a separate
    function exists for shutting down the sub-devices representing each of the PS/2 ports.
    This function powers down the PS/2 ports and clears the controller's global interrupt enable
    flag.
*/
s32 ps2controller_shut_down(dev_t *dev)
{
    /* Disable interrupts and deactivate power to both ports */
    PS2CTRLR_REG(dev->base_addr, PS2_CFG) &= ~(PS2_CFG_PWR_A | PS2_CFG_PWR_B | PS2_CFG_IE);

    if(dev->data != NULL)
    {
        kfree(dev->data);
        dev->data = NULL;
    }

    return SUCCESS;
}


/*
    ps2controller_port_shut_down() - prepare for deletion of the port device.  This function
    disables interrupts from the port.
*/
s32 ps2controller_port_shut_down(dev_t *dev)
{
    ps2controller_port_state_t * const state = (ps2controller_port_state_t *) dev->data;
    if(state != NULL)
    {
        /* Disable all interrupts from the port */
        *state->regs.int_cfg = 0;

        cpu_irq_remove_handler(dev->irql, ps2controller_port_irq_handler, dev->data);

        kfree(dev->data);
        dev->data = NULL;
    }

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
