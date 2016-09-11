/*
    ATmega8-driven dual PS/2 port controller driver

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, December 2015.

    This device has driver ID 0x82.
*/

#include <driver/ps2controller.h>
#include <kernel/cpu.h>
#include <kernel/include/keyboard.h>
#include <klibc/stdio.h>			/* TODO remove */


void ps2controller_port_irq_handler(ku32 irql, void *data);
void ps2controller_port_start_tx(ps2controller_port_state_t *state);
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
    PS/2 scan code set 2 single-byte scan code to internal key code map.  This table maps single-
    byte scan codes (i.e. those without a prefix byte like 0xe0 or 0xe1) to internal key codes.
*/
const KeyCode ps2_sc2_to_internal[] =
{
    KEY_NONE,       KEY_F9,         KEY_NONE,       KEY_F5,
    KEY_F3,         KEY_F1,         KEY_F2,         KEY_F12,
    KEY_NONE,       KEY_F10,        KEY_F8,         KEY_F6,
    KEY_F4,         KEY_TAB,        KEY_BKTICK,     KEY_NONE,

    KEY_NONE,       KEY_ALT_L,      KEY_SHIFT_L,    KEY_NONE,
    KEY_CTRL_L,     KEY_Q,          KEY_1,          KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_Z,          KEY_S,
    KEY_A,          KEY_W,          KEY_2,          KEY_NONE,

    KEY_NONE,       KEY_C,          KEY_X,          KEY_D,
    KEY_E,          KEY_4,          KEY_3,          KEY_NONE,
    KEY_NONE,       KEY_SPACE,      KEY_V,          KEY_F,
    KEY_T,          KEY_R,          KEY_5,          KEY_NONE,

    KEY_NONE,       KEY_N,          KEY_B,          KEY_H,
    KEY_G,          KEY_Y,          KEY_6,          KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_M,          KEY_J,
    KEY_U,          KEY_7,          KEY_8,          KEY_NONE,

    KEY_NONE,       KEY_COMMA,      KEY_K,          KEY_I,
    KEY_O,          KEY_0,          KEY_9,          KEY_NONE,
    KEY_NONE,       KEY_DOT,        KEY_FWDSL,      KEY_L,
    KEY_SCOLON,     KEY_P,          KEY_MINUS,      KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_APOS,       KEY_NONE,
    KEY_BKT_L,      KEY_EQUALS,     KEY_NONE,       KEY_NONE,
    KEY_CAPS,       KEY_SHIFT_R,    KEY_ENTER,      KEY_BKT_R,
    KEY_NONE,       KEY_HASH,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_BKSLASH,    KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_BKSPACE,    KEY_NONE,
    KEY_NONE,       KEY_NP_1,       KEY_NONE,       KEY_NP_4,
    KEY_NP_7,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NP_0,       KEY_NP_DOT,     KEY_NP_2,       KEY_NP_5,
    KEY_NP_6,       KEY_NP_8,       KEY_ESC,        KEY_NUM,
    KEY_F11,        KEY_NP_PLUS,    KEY_NP_3,       KEY_NP_MINUS,
    KEY_NP_STAR,    KEY_NP_9,       KEY_SCROLL,     KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_F7,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
};


/*
    PS/2 scan code set 2 "0xe0-prefixed two-byte scan code" to internal key code map.  This table
    maps two-byte scan codes, starting with an implicit 0xe0 byte, to internal key codes.
*/
const KeyCode ps2_sc2_ext1_to_internal[] =
{
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_ALT_R,      KEY_NONE,       KEY_NONE,
    KEY_CTRL_R,     KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_GUI_L,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_GUI_R,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_APPS,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_POWER,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_SLEEP,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NP_FWDSL,   KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NP_ENTER,   KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_WAKE,       KEY_NONE,

    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_END,        KEY_NONE,       KEY_ARROW_L,
    KEY_HOME,       KEY_NONE,       KEY_NONE,       KEY_NONE,

    KEY_INSERT,     KEY_DELETE,     KEY_ARROW_D,    KEY_NONE,
    KEY_ARROW_R,    KEY_ARROW_U,    KEY_NONE,       KEY_NONE,
    KEY_NONE,       KEY_NONE,       KEY_PG_DOWN,    KEY_NONE,
    KEY_NONE,       KEY_PG_UP,      KEY_NONE,       KEY_NONE,
};


/*
    PS/2 scan code set 2 single-byte scan code bitmap.  In this bitmap, a bit is set when the
    corresponding single-byte scan-code from PS/2 scan code set 2 is valid.  The test is carried out
    like this:

        if(ps2_sc2_single_byte_code_map[scan_code >> 3] & (1 << (scan_code & 0x7)))
            valid = 1;
*/
#if 0  /* Not currently used */
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
#endif


/*
    PS/2 scan code set 2 two-byte scan code bitmap.  In this bitmap, a bit is set when the second
    byte in the scan code sequence 0xe0, 0xXX is valid.  Note that this bitmap only covers codes
    below 0x80, as all higher code points are vacant.  The test is carried out like this:

        if((second_byte < 0x80) &&
           (ps2_sc2_ext1_code_map[second_byte >> 3] & (1 << (second_byte & 0x7)))
            valid = 1;
*/
#if 0   /* Not currently used */
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
#endif


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
    *state->regs.int_cfg = PS2_FLAG_RX | PS2_FLAG_TX | PS2_FLAG_PAR_ERR | PS2_FLAG_OVF;

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
                state->packet.flags |= PS2_PKT_KB_EXT1;
            else if(key == PS2_SC_KB_EXT2)
                state->packet.flags |= PS2_PKT_KB_EXT2;
            else if(key == PS2_SC_KB_RELEASE)
                state->packet.flags |= PS2_PKT_KB_RELEASE;
            else
            {
                /* Accumulate the received scan code into the keyboard data buffer */
                state->packet.data = (state->packet.data << 8) | key;

                ps2controller_process_key(state);
            }
        }

        if(status & PS2_FLAG_TX)
        {
            /* Transmit complete; send the next byte, if any */
            if(!CIRCBUF_IS_EMPTY(state->tx_buf))
                *state->regs.data = CIRCBUF_READ(state->tx_buf);
            else
                state->tx_in_progress = 0;      /* Transmit finished */
        }

        if(status & PS2_ERR_MASK)
            state->err |= status & PS2_ERR_MASK;

        *state->regs.status = 0;
    }
}


/*
    ps2controller_port_start_tx(): if the associated transmit buffer is not empty, start
    transmission of data on a PS/2 port.  If transmission is already in progress, this function has
    no effect.  If multiple bytes are queued for transmission, this function will ensure that
    transmission is scheduled to occur automatically.
*/
void ps2controller_port_start_tx(ps2controller_port_state_t *state)
{
    /* Disable "TX Done" interrupt on the port */
    *state->regs.int_cfg &= ~PS2_FLAG_TX;

    /* At this point, it is safe to test the value of the the tx_in_progress flag. */
    if(!state->tx_in_progress && !CIRCBUF_IS_EMPTY(state->tx_buf))
    {
        state->tx_in_progress = 1;
        *state->regs.data = CIRCBUF_READ(state->tx_buf);
    }

    /* Enable "TX Done" interrupt on the port */
    *state->regs.int_cfg |= PS2_FLAG_TX;
}


void ps2controller_process_key(ps2controller_port_state_t *state)
{
    ku32 data = state->packet.data;
    ku8 flags = state->packet.flags & (PS2_PKT_KB_EXT1 | PS2_PKT_KB_EXT2),
        release = state->packet.flags & PS2_PKT_KB_RELEASE,
        scan_code = data & 0xff;
    u8 key_code;

    if(flags == 0)
    {
        /* If neither of the EXTx flags is true, this must be a single-byte code */
        key_code = ps2_sc2_to_internal[scan_code];
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
            key_code = ps2_sc2_ext1_to_internal[scan_code];

            if(!release)
            {
                if(scan_code == (PS2_SC2_PRTSC_PRESS >> 8))
                    return;     /* First byte of a PrtSc press - wait for next byte */
                else if(data == PS2_SC2_PRTSC_PRESS)
                    key_code = KEY_PRT_SC;
            }
            else
            {
                if(scan_code == (PS2_SC2_PRTSC_RELEASE >> 8))
                    return;     /* First byte of a PrtSc release - wait for next byte */
                else if(data == PS2_SC2_PRTSC_RELEASE)
                    key_code = KEY_PRT_SC;
            }
        }
        else
            key_code = KEY_NONE;    /* Invalid sequence */
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

        if(release && (data == PS2_SC2_PAUSE))
            key_code = KEY_PAUSE;
        else
            key_code = KEY_NONE;
    }
    else
        key_code = KEY_NONE;    /* Ignore if both PS2_PKT_KB_EXT1 & PS2_PKT_KB_EXT2 are set */

    /* Do stuff here */
    switch(key_code)
    {
        case KEY_SHIFT_L:
        case KEY_SHIFT_R:
            if(!release)
                state->state.kb.modifiers |= KMF_SHIFT;
            else
                state->state.kb.modifiers &= ~KMF_SHIFT;
            break;

        case KEY_CTRL_L:
        case KEY_CTRL_R:
            if(!release)
                state->state.kb.modifiers |= KMF_CTRL;
            else
                state->state.kb.modifiers &= ~KMF_CTRL;
            break;

        case KEY_ALT_L:
        case KEY_ALT_R:
            if(!release)
                state->state.kb.modifiers |= KMF_ALT;
            else
                state->state.kb.modifiers &= ~KMF_ALT;
            break;

        case KEY_CAPS:
        case KEY_NUM:
        case KEY_SCROLL:
            if(release)
            {
                u8 * const modifiers = &state->state.kb.modifiers;
                u8 * const leds = &state->state.kb.leds;

                switch(key_code)
                {
                    case KEY_CAPS:
                        *modifiers ^= KMF_CAPS;
                        *leds ^= PS2_KB_LED_CAPS;
                        break;

                    case KEY_NUM:
                        *modifiers ^= KMF_NUM;
                        *leds ^= PS2_KB_LED_NUM;
                        break;

                    case KEY_SCROLL:
                        *modifiers ^= KMF_SCROLL;
                        *leds ^= PS2_KB_LED_SCROLL;
                        break;
                }

                CIRCBUF_WRITE(state->tx_buf, PS2_CMD_SET_LEDS);
                CIRCBUF_WRITE(state->tx_buf, *leds);
                ps2controller_port_start_tx(state);
            }
            break;

        default:
            if(!release)
                putchar(keymap_get(key_code, state->state.kb.modifiers));
            break;
    }

    /* Either the scan code was handled, or it was invalid.  Reset state here. */
    state->packet.data = 0;
    state->packet.flags = 0;
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
