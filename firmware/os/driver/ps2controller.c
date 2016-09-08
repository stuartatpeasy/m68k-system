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

void ps2controller_process_key(ps2controller_port_state_t *state);      // FIXME


/*
    PS/2 scan code set 2 single-byte scan code bitmap.  In this bitmap, a bit is set when the
    corresponding single-byte scan-code from PS/2 scan code set 2 is valid.  The test is carried out
    like this:

        if(ps2_sc2_single_byte_code_map[scan_code >> 3] & (1 << (scan_code & 0x7)))
            valid = 1;
*/
const u8 ps2_sc2_single_byte_code_map[32] =
{
    /* bit       0   1   2   3   4   5   6   7  */
    0xfa,   /*      01      03  04  05  06  07  */
    0x7e,   /*      09  0a  0b  0c  0d  0e      */
    0x76,   /*      11  12      14  15  16      */
    0x7c,   /*          1a  1b  1c  1d  1e      */
    0x7e,   /*      21  22  23  24  25  26      */
    0x7e,   /*      29  2a  2b  2c  2d  2e      */
    0x7e,   /*      31  32  33  34  35  36      */
    0x7c,   /*          3a  3b  3c  3d  3e      */
    0x7e,   /*      41  42  43  44  45  46      */
    0x7e,   /*      49  4a  4b  4c  4d  4e      */
    0x34,   /*          52      54  55          */
    0x2f,   /*  58  59  5a  5b      5d          */
    0x40,   /*                          66      */
    0x1a,   /*      69      6b  6c              */
    0xff,   /*  70  71  72  73  74  75  76  77  */
    0x7f,   /*  78  79  7a  7b  7c  7d  7e      */
    0x08,   /*              83                  */
    0x00,   /*                                  */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/*
    PS/2 scan code set 2 two-byte scan code bitmap.  In this bitmap, a bit is set when the second
    byte in the scan code sequence 0xe0, 0xXX is valid.  Note that this bitmap only covers codes
    below 0x80, as all higher code points are vacant.  The test is carried out like this:

        if((second_byte < 0x80) &&
           (ps2_sc2_ext1_code_map[second_byte >> 3] & (1 << (second_byte & 0x7)))
            valid = 1;
*/
const u8 ps2_sc2_ext1_code_map[16] =
{
    /* bit       0   1   2   3   4   5   6   7  */
    0x00,   /*                                  */
    0x00,   /*                                  */
    0x12,   /*      11          14              */
    0x80,   /*                              1f  */
    0x80,   /*                              27  */
    0x80,   /*                              2f  */
    0x80,   /*                              37  */
    0x80,   /*                              3f  */
    0x00,   /*                                  */
    0x04,   /*          4a                      */
    0x00,   /*                                  */
    0x44,   /*          5a              5e      */
    0x00,   /*                                  */
    0x1a,   /*      69      6b  6c              */
    0x37,   /*  70  71  72      74  75          */
    0x24,   /*          7a          7d          */
};


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
                /* Accumulate the received scan code into the keyboard data buffer */
                state->packet.kb.data = (state->packet.kb.data << 8) | key;

                ps2controller_process_key(state);
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
    ku32 data = state->packet.kb.data;
    ku8 flags = state->packet.kb.flags & (PS2_PKT_KB_EXT1 | PS2_PKT_KB_EXT2),
        release = state->packet.kb.flags & PS2_PKT_KB_RELEASE,
        scan_code = data & 0xff;

    if(flags == 0)
    {
        /* If neither of the EXTx flags is true, this must be a single-byte code */
        if(ps2_sc2_single_byte_code_map[scan_code >> 3] & (1 << (scan_code & 0x7)))
        {
            if(!release)
                printf("[%02x]", scan_code);
            else
                printf("[!%02x]", scan_code);
        }
    }
    else if(flags == PS2_PKT_KB_EXT1)
    {
        /*
            Most EXT1-prefixed keys are single-byte codes; two aren't.  We see
            "PrtSc pressed" as 0x127c, and "PrtSc released" as 0x7c12", with the
            appropriate flags set.  We defer processing the key if we have received
            only the first byte of either of these two (0x127c / 0x7c12) scan codes.
            Otherwise, the scan code is processed now.
        */
        if(scan_code < 0x80)
        {
            if(ps2_sc2_ext1_code_map[scan_code >> 3] & (1 << (scan_code & 0x7)))
            {
                /* Boom. */
                if(!release)
                    printf("[1:%02x]", scan_code);
                else
                    printf("[!1:%02x]", scan_code);
            }
            else if(!release)
            {
                if(scan_code == 0x12)
                    return;     /* First byte of a PrtSc press - wait for next byte */
                else if(data == 0x127c)
                    put("[prtsc]");
            }
            else if(release)
            {
                if(scan_code == 0x7c)
                    return;     /* First byte of a PrtSc release - wait for next byte */
                else if(data == 0x7c12)
                    put("[!prtsc]");
            }
        }
    }
    else if(flags == PS2_PKT_KB_EXT2)
    {
        /*
            The only EXT2-prefixed code we recognise is "Pause pressed", which we
            receive as 0x14771477.  The scan code is actually 0xe1, 0x14, 0x77,
            0xe1, 0xf0, 0x14, 0xf0, 0x77.  Note that misplaced prefixes will not be
            detected.  If we have received any four bytes (not including the prefix
            characters 0xe1 and 0xf0), we process the key.
        */
        if(!(data & 0xff000000))
            return;     /* Wait for another code to arrive */

        if(release && (data == 0x14771477))
            put("[pause]");
    }
    /* Note: we ignore sequences where both PS2_PKT_KB_EXT1 and PS2_PKT_KB_EXT2 are set */

    /* Either the scan code was handled, or it was invalid.  Reset state here. */
    state->packet.kb.data = 0;
    state->packet.kb.flags = 0;
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
