/*
	MC68681 DUART "driver"

	(c) Stuart Wallace, December 2011.


    NOTE: these two functions are defined inline in mc68681.h:
            inline void mc68681_start_counter(dev_t *dev, ku16 init_count);
            inline void mc68681_stop_counter(dev_t *dev);
*/

#include <driver/mc68681.h>
#include <kernel/cpu.h>
#include <kernel/memory/kmalloc.h>


static void mc68681_set_brg(dev_t *dev, ku8 brg_set, ku8 brg_test);
static void mc68681_set_ct_mode(dev_t *dev, ku8 mode);

void mc68681_irq_handler(ku32 irql, void *arg);

const mc68681_baud_rate_entry g_mc68681_baud_rates[22] =
{
    {50,        0x00},
    {75,        0x10},
    {110,       0x01},
    {134,       0x02},
    {150,       0x13},
    {200,       0x03},
    {300,       0x04},
    {600,       0x05},
    {1050,      0x07},
    {1200,      0x06},
    {1800,      0x1a},
    {2000,      0x17},
    {2400,      0x08},
    {4800,      0x09},
    {7200,      0x0a},
    {9600,      0x0b},
    {14400,     0x33},
    {19200,     0x1c},
    {28800,     0x24},
    {38400,     0x0c},
    {57600,     0x25},
    {115200,    0x26},
};


/*
    mc68681_init() - initialise the MC68681 DUART
*/
s32 mc68681_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    mc68681_state_t *state = CHECKED_KCALLOC(1, sizeof(mc68681_state_t));

    CIRCBUF_INIT(state->rxa_buf);
    CIRCBUF_INIT(state->txa_buf);
    CIRCBUF_INIT(state->rxb_buf);
    CIRCBUF_INIT(state->txb_buf);

    dev->data = state;

    mc68681_reset(base_addr);

    state->imr = 0x00;
    MC68681_REG(base_addr, MC68681_IMR) = state->imr;    /* Disable all interrupts during init */

	/* Set mode register 1A */
	MC68681_REG(base_addr, MC68681_MRA) = /* 0x93 */
        BIT(MC68681_MR1_RXRTS) |                                        /* Enable RX RTS        */
        (MC68681_PARITY_MODE_NONE << MC68681_MR1_PARITY_MODE_SHIFT) |   /* No parity            */
        (MC68681_BPC_8 << MC68681_MR1_BPC_SHIFT);                       /* 8 bits per character */

	/*
        Set mode register 2A

        Note: the MC68681 uses  the same address for MR1A and MR2A.  Having written to MR1A (above),
        an internal pointer in the IC switches such that the next access will address MR2A.
    */
	MC68681_REG(base_addr, MC68681_MRA) = /* 0x17 */
        (MC68681_CHAN_MODE_NORMAL << MC68681_MR2_CHAN_MODE_SHIFT) |
        BIT(MC68681_MR2_CTS) |
        (MC68681_STOP_BIT_1_000 << MC68681_MR2_STOP_BIT_LEN_SHIFT);

    /* Set mode register 1B */
    MC68681_REG(base_addr, MC68681_MRB) = /* 0x93 */
        BIT(MC68681_MR1_RXRTS) |                                        /* Enable RX RTS        */
        (MC68681_PARITY_MODE_NONE << MC68681_MR1_PARITY_MODE_SHIFT) |   /* No parity            */
        (MC68681_BPC_8 << MC68681_MR1_BPC_SHIFT);                       /* 8 bits per character */

    /* Set mode register 2B (see notes for mode register 2A, above) */
	MC68681_REG(base_addr, MC68681_MRB) = /* 0x17 */
        (MC68681_CHAN_MODE_NORMAL << MC68681_MR2_CHAN_MODE_SHIFT) |
        BIT(MC68681_MR2_CTS) |
        (MC68681_STOP_BIT_1_000 << MC68681_MR2_STOP_BIT_LEN_SHIFT);

    /* Set baud rate generator clock source to the external crystal clock divided by 16 */
    mc68681_set_ct_mode(dev, MC68681_CT_MODE_C_XTAL16);             /* BRG source = xtal/16 */

	/* Enable the channel A transmitter and receiver */
	MC68681_REG(base_addr, MC68681_CRA) = /* 0x05 */
        (MC68681_CMD_TX_ENABLE << MC68681_CR_TX_CMD_SHIFT) |
        (MC68681_CMD_RX_ENABLE << MC68681_CR_RX_CMD_SHIFT);

	/* Enable the channel B transmitter and receiver */
	MC68681_REG(base_addr, MC68681_CRB) = /* 0x05 */
        (MC68681_CMD_TX_ENABLE << MC68681_CR_TX_CMD_SHIFT) |
        (MC68681_CMD_RX_ENABLE << MC68681_CR_RX_CMD_SHIFT);

    /* Set output control register to defaults (all pins = general-purpose outputs) */
    state->opcr = 0x00;
	MC68681_REG(base_addr, MC68681_OPCR) = state->opcr;

	/*
		Set OPR - output port bits
		Each bit in the OPR must be set to the complement of the required output pin level.
	*/
	MC68681_REG(base_addr, MC68681_SOPR) = 0xff;
	MC68681_REG(base_addr, MC68681_ROPR) = 0x00;

	/* Set channel A baud rate to 115200 */
	s32 ret = mc68681_set_baud_rate(dev, MC68681_CHANNEL_A, 115200);

    cpu_irq_add_handler(dev->irql, dev, mc68681_irq_handler);

    return ret;
}


/*
    mc68681_set_output_pin_fn() - configure output pin functions
*/
s32 mc68681_set_output_pin_fn(dev_t *dev, const mc68681_output_pin_t pin, const mc68681_pin_fn_t fn)
{
    u8 *opcr = &(((mc68681_state_t *) dev->data)->opcr);

	/*
		Set OPCR - output port function select
			bit		val		desc
		----------------------------------------------------------------------
			7		0		OP7 - 0: complement of OPR7; 1: TxRDYB interrupt
			6		0		OP6 - 0: complement of OPR6; 1: TxRDYA interrupt
			5		0		OP5 - 0: complement of OPR5; 1: RxB/FFULLB interrupt
			4		0		OP4 - 0: complement of OPR4; 1: RxA/FFULLA interrupt
			3		0		} OP3 - 00: complement of OPR3; 01: C/T output
			2		1		}       10: ch B Tx clk; 11: ch B Rx clk
			1		0		} OP2 - 00: complement of OPR2; 01: ch A Tx 16x clk
			0		0		}       10: ch A Tx 1x clk; 11: ch A Rx 1x clk

        OP1 and OP0 are always GPIOs.
	*/
	if(fn == mc68681_pin_fn_gpio)
    {
        if(pin >= mc68681_pin_op4)
            *opcr &= ~BIT(pin);
        else if(pin == mc68681_pin_op3)
            *opcr &= ~(BIT(3) | BIT(2));
        else if(pin == mc68681_pin_op2)
            *opcr &= ~(BIT(1) | BIT(0));

        /* Fall through for mc68681_pin_op[1|0] -- these pins are always GPIOs. */
    }
    else
    {
        if(pin == mc68681_pin_op3)
        {
            if(fn == mc68681_pin_fn_ct_output)      /* OP3 -> counter/timer output */
            {
                *opcr &= ~BIT(3);
                *opcr |= BIT(2);
            }
            else if(fn == mc68681_pin_fn_txb_clk)   /* OP3 -> TxB clock             */
            {
                *opcr |= BIT(3);
                *opcr &= ~BIT(2);
            }
            else if(fn == mc68681_pin_fn_rxb_clk)   /* OP3 -> RxB clock             */
                *opcr |= BIT(3) | BIT(2);
            else
                return EINVAL;
        }
        else if(pin == mc68681_pin_op2)
        {
            if(fn == mc68681_pin_fn_txa16_clk)      /* OP2 -> TxA 16x clock         */
            {
                *opcr &= ~BIT(1);
                *opcr |= BIT(0);
            }
            else if(fn == mc68681_pin_fn_txa_clk)   /* OP2 -> TxA clock             */
            {
                *opcr |= BIT(1);
                *opcr &= ~BIT(0);
            }
            else if(fn == mc68681_pin_fn_rxa_clk)   /* OP2 -> RxA clock             */
                *opcr |= BIT(1) | BIT(0);
            else
                return EINVAL;
        }
        else
            return EINVAL;
    }

    MC68681_REG(dev->base_addr, MC68681_OPCR) = *opcr;
    return SUCCESS;
}


/*
    mc68681_serial_a_init() - device initialiser for serial channel A.
*/
s32 mc68681_serial_a_init(dev_t *dev)
{
    mc68681_state_t *state;

    dev->data = dev->parent->data;

    dev->getc = mc68681_channel_a_getc;
    dev->putc = mc68681_channel_a_putc;
    dev->block_size = 1;
    dev->len = 1;

    state = (mc68681_state_t *) dev->data;

    /* Enable serial port A "receiver ready" interrupt */
    state->imr |= BIT(MC68681_IMR_RXRDY_FFULL_A) | BIT(MC68681_IMR_TXRDY_A);
    MC68681_REG(dev->base_addr, MC68681_IMR) = state->imr;

    return SUCCESS;
}


/*
    mc68681_serial_b_init() - device initialiser for serial channel B.
*/
s32 mc68681_serial_b_init(dev_t *dev)
{
    mc68681_state_t *state;

    dev->data = dev->parent->data;

    dev->getc = mc68681_channel_b_getc;
    dev->putc = mc68681_channel_b_putc;
    dev->block_size = 1;
    dev->len = 1;

    state = (mc68681_state_t *) dev->parent->data;

    /* Enable serial port B "receiver ready" interrupt */
    state->imr |= BIT(MC68681_IMR_RXRDY_FFULL_B) | BIT(MC68681_IMR_TXRDY_B);
    MC68681_REG(dev->base_addr, MC68681_IMR) = state->imr;

    return SUCCESS;
}


/*
    mc68681_irq_handler() - handle interrupts generated by the MC68681.
*/
void mc68681_irq_handler(ku32 irql, void *arg)
{
    dev_t * const dev = (dev_t *) arg;
    void * const base_addr = dev->base_addr;
    mc68681_state_t *state = (mc68681_state_t *) dev->data;
    UNUSED(irql);

    ku8 irq_status = MC68681_REG(base_addr, MC68681_ISR);

    /* Serial channel A "receiver ready" interrupt */
    if(irq_status & BIT(MC68681_IMR_RXRDY_FFULL_A))
    {
        do
        {
            /*
                Transfer a byte from the receive FIFO to the RX buffer.  Note: if the buffer is
                full, the character will be dropped.
            */
            ku8 data = MC68681_REG(base_addr, MC68681_RHRA);
            if(!CIRCBUF_IS_FULL(state->rxa_buf))
                CIRCBUF_WRITE(state->rxa_buf, data);
        }
        while(MC68681_REG(base_addr, MC68681_ISR) & BIT(MC68681_IMR_RXRDY_FFULL_A));
    }

    /* Serial channel A "transmitter ready" interrupt */
    if(irq_status & BIT(MC68681_IMR_TXRDY_A))
    {
        /*
            Transmitter ready.  If there is another character to transmit, do so; if not, disable
            this interrupt.
        */
        if(!CIRCBUF_IS_EMPTY(state->txa_buf))
            MC68681_REG(base_addr, MC68681_THRA) = CIRCBUF_READ(state->txa_buf);
        else
        {
            state->imr &= ~BIT(MC68681_IMR_TXRDY_A);
            MC68681_REG(base_addr, MC68681_IMR) = state->imr;
        }
    }

    /* Serial channel B "receiver ready" interrupt */
    if(irq_status & BIT(MC68681_IMR_RXRDY_FFULL_B))
    {
        do
        {
            /*
                Transfer a byte from the receive FIFO to the RX buffer.  Note: if the buffer is
                full, the character will be dropped.
            */
            CIRCBUF_WRITE(state->rxb_buf, MC68681_REG(base_addr, MC68681_RHRB));
        }
        while(MC68681_REG(base_addr, MC68681_ISR) & BIT(MC68681_IMR_RXRDY_FFULL_B));
    }

    /* Serial channel B "transmitter ready" interrupt */
    if(irq_status & BIT(MC68681_IMR_TXRDY_B))
    {
        /*
            Transmitter ready.  If there is another character to transmit, do so; if not, disable
            this interrupt.
        */
        if(!CIRCBUF_IS_EMPTY(state->txb_buf))
            MC68681_REG(base_addr, MC68681_THRA) = CIRCBUF_READ(state->txb_buf);
        else
        {
            state->imr &= ~BIT(MC68681_IMR_TXRDY_B);
            MC68681_REG(base_addr, MC68681_IMR) = state->imr;
        }
    }
}


/*
    mc68681_shut_down() - shut down the main MC68681 driver device.
    NOTE: this will invalidate the child devices.
*/
s32 mc68681_shut_down(dev_t *dev)
{
    MC68681_REG(dev->base_addr, MC68681_IMR) = 0;   /* Disable all MC68681 interrupts */
    kfree(dev->data);

    return SUCCESS;
}


/*
    mc68681_control() - handler for device control requests
*/
s32 mc68681_control(dev_t *dev, ku32 channel, const devctl_fn_t fn, const void *in, void *out)
{
    switch(fn)
    {
        case dc_get_baud_rate:
            *((u32 *) out) = mc68681_get_baud_rate(dev, channel);
            return SUCCESS;

        case dc_set_baud_rate:
            return mc68681_set_baud_rate(dev, channel, *((u32 *) in));

        default:
            return ENOSYS;
    }
}


/*
    mc68681_set_brg() - select a baud-rate generator.  brg_set specifies which of baud-rate sets 0
    and 1 should be used; a nonzero brg_test arg specifies that the "test" baud rates should be
    used.
*/
static void mc68681_set_brg(dev_t *dev, ku8 brg_set, ku8 brg_test)
{
    /* TODO - mutex */
    void * const base_addr = dev->base_addr;
    mc68681_state_t *s = (mc68681_state_t *) dev->data;

    if(brg_set)
        s->acr |= BIT(MC68681_ACR_BRG_SELECT);
    else
        s->acr &= ~BIT(MC68681_ACR_BRG_SELECT);

    MC68681_REG(base_addr, MC68681_ACR) = s->acr;

    if((brg_test && !s->brg_test) || (!brg_test && s->brg_test))
    {
        u8 dummy = MC68681_REG(base_addr, MC68681_BRG_TEST);
        dummy += 0;     /* silence "set but not used" compiler warning */
        s->brg_test = ~s->brg_test;
    }
}


/*
    mc68681_set_ct_mode() - set the mode and clock source for the MC68681 counter/timer.  This has
    to be done carefully, as the MC68681 register involved (ACR) is write-only.
*/
static void mc68681_set_ct_mode(dev_t *dev, ku8 mode)
{
    /* TODO - mutex */
    mc68681_state_t *s = (mc68681_state_t *) dev->data;

    s->acr &= ~(MC68681_ACR_CT_MODE_MASK << MC68681_ACR_CT_MODE_SHIFT);
    s->acr |= (mode & MC68681_ACR_CT_MODE_MASK) << MC68681_ACR_CT_MODE_SHIFT;

    MC68681_REG(dev->base_addr, MC68681_ACR) = s->acr;
}


/*
    mc68681_set_baud_rate() - set the baud rate for the specified channel.

    NOTE: both channels use the same baud-rate generator.  If a channel is changed to a rate not
    available in the current "baud-rate set", a new baud-rate set will be selected.  This will
    affect the baud rate of the other channel!  See the MC/SCC/SCN68681 data sheet for more
    information.
*/
s32 mc68681_set_baud_rate(dev_t *dev, ku16 channel, ku32 rate)
{
    const mc68681_baud_rate_entry *p;
    mc68681_state_t *state = (mc68681_state_t *) dev->data;

    if(channel > MC68681_CHANNEL_B)
        return EINVAL;  /* 0 = channel A; 1 = channel B; anything else = invalid */

    for(p = g_mc68681_baud_rates; p < &(g_mc68681_baud_rates[ARRAY_COUNT(g_mc68681_baud_rates)]);
        ++p)
    {
        if(rate == p->rate)
        {
            mc68681_set_brg(dev, p->csr & MC68681_BRE_ACR7, p->csr & MC68681_BRE_TEST);

            if(channel == MC68681_CHANNEL_A)    /* Channel A */
            {
                MC68681_REG(dev->base_addr, MC68681_CSRA) =
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_RXCLK_SHIFT) |
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_TXCLK_SHIFT);
                state->baud_a = rate;
            }
            else                                /* Channel B */
            {
                MC68681_REG(dev->base_addr, MC68681_CSRB) =
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_RXCLK_SHIFT) |
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_TXCLK_SHIFT);
                state->baud_b = rate;
            }

            return SUCCESS;
        }
    }

    return EINVAL;      /* No such baud rate */
}


/*
    mc68681_get_baud_rate() - get the current baud rate for the specified serial channel.
*/
u32 mc68681_get_baud_rate(dev_t *dev, ku16 channel)
{
    if(channel == MC68681_CHANNEL_A)
        return ((mc68681_state_t *) dev->data)->baud_a;
    else if(channel == MC68681_CHANNEL_B)
        return ((mc68681_state_t *) dev->data)->baud_b;
    else
        return 0;
}


/*
    mc68681_reset_rx() - reset MC68681 receiver
*/
s32 mc68681_reset_rx(void * const base_addr, ku16 channel)
{
    if(channel > MC68681_CHANNEL_B)
        return EINVAL;

    if(channel == MC68681_CHANNEL_A)        /* Channel A */
        MC68681_REG(base_addr, MC68681_CRA) = /* 0x20 */
            (MC68681_CMD_RESET_RX << MC68681_CR_MISC_CMD_SHIFT);
    else                                    /* Channel B */
        MC68681_REG(base_addr, MC68681_CRB) = /* 0x20 */
            (MC68681_CMD_RESET_RX << MC68681_CR_MISC_CMD_SHIFT);

    return SUCCESS;
}


/*
    mc68681_reset_tx() - reset MC68681 transmitter
*/
s32 mc68681_reset_tx(void * const base_addr, ku16 channel)
{
    if(channel > MC68681_CHANNEL_B)
        return EINVAL;

    if(channel == MC68681_CHANNEL_A)        /* Channel A */
        MC68681_REG(base_addr, MC68681_CRA) = /* 0x30 */
            (MC68681_CMD_RESET_TX << MC68681_CR_MISC_CMD_SHIFT);
    else
        MC68681_REG(base_addr, MC68681_CRB) = /* 0x30 */
            (MC68681_CMD_RESET_TX << MC68681_CR_MISC_CMD_SHIFT);

    return SUCCESS;
}


/*
    mc68681_reset() - perform a device reset on the MC68681.  Note: does not reset RX or TX
*/
void mc68681_reset(void * const base_addr)
{
    /* Send some initialisation commands to the MC68681 */
    MC68681_REG(base_addr, MC68681_CRA) =  /* 0x1a */
        (MC68681_CMD_RESET_MR_PTR << MC68681_CR_MISC_CMD_SHIFT) |   /* Reset MRA pointer        */
        (MC68681_CMD_TX_DISABLE << MC68681_CR_TX_CMD_SHIFT) |       /* Disable channel A TX     */
        (MC68681_CMD_RX_DISABLE << MC68681_CR_RX_CMD_SHIFT);        /* Disable channel A RX     */

    MC68681_REG(base_addr, MC68681_CRB) =  /* 0x1a */
        (MC68681_CMD_RESET_MR_PTR << MC68681_CR_MISC_CMD_SHIFT) |   /* Reset MRA pointer        */
        (MC68681_CMD_TX_DISABLE << MC68681_CR_TX_CMD_SHIFT) |       /* Disable channel B TX     */
        (MC68681_CMD_RX_DISABLE << MC68681_CR_RX_CMD_SHIFT);        /* Disable channel B RX     */

    mc68681_reset_tx(base_addr, MC68681_CHANNEL_A);
    mc68681_reset_tx(base_addr, MC68681_CHANNEL_B);
    mc68681_reset_rx(base_addr, MC68681_CHANNEL_A);
    mc68681_reset_rx(base_addr, MC68681_CHANNEL_B);
}


/*
    mc68681_channel_a_set_baud_rate() - set the baud rate for serial channel A.
    Note: see the notes for mc68681_set_baud_rate(), above.
*/
s32 mc68681_channel_a_set_baud_rate(dev_t *dev, ku32 rate)
{
    return mc68681_set_baud_rate(dev, MC68681_CHANNEL_A, rate);
}


/*
    mc68681_channel_a_get_baud_rate() - get the current baud rate for serial channel A.
*/
u32 mc68681_channel_a_get_baud_rate(dev_t *dev)
{
    return mc68681_get_baud_rate(dev, MC68681_CHANNEL_A);
}


/*
    mc68681_channel_b_set_baud_rate() - set the baud rate for serial channel B.
    Note: see the notes for mc68681_set_baud_rate(), above.
*/
s32 mc68681_channel_b_set_baud_rate(dev_t *dev, ku32 rate)
{
    return mc68681_set_baud_rate(dev, MC68681_CHANNEL_B, rate);
}


/*
    mc68681_channel_b_get_baud_rate() - get the current baud rate for serial channel B.
*/
u32 mc68681_channel_b_get_baud_rate(dev_t *dev)
{
    return mc68681_get_baud_rate(dev, MC68681_CHANNEL_B);
}


/*
    mc68681_channel_a_putc() - write a character to serial channel A, blocking until done.
*/
s32 mc68681_channel_a_putc(dev_t *dev, const char c)
{
    mc68681_state_t * const state = (mc68681_state_t *) dev->parent->data;

    /* FIXME: return EAGAIN if FIFO is full? */
    while(CIRCBUF_IS_FULL(state->txa_buf))
        ;

    CIRCBUF_WRITE(state->txa_buf, c);
    state->imr |= BIT(MC68681_IMR_TXRDY_A);
    MC68681_REG(dev->base_addr, MC68681_IMR) = state->imr;

    return SUCCESS;
}


/*
    mc68681_channel_b_putc() - write a character to serial channel B, blocking until done.
*/
s32 mc68681_channel_b_putc(dev_t *dev, const char c)
{
    mc68681_state_t * const state = (mc68681_state_t *) dev->parent->data;

    /* FIXME: return EAGAIN if FIFO is full? */
    while(CIRCBUF_IS_FULL(state->txb_buf))
        ;

    CIRCBUF_WRITE(state->txb_buf, c);
    state->imr |= BIT(MC68681_IMR_TXRDY_B);
    MC68681_REG(dev->base_addr, MC68681_IMR) = state->imr;

    return SUCCESS;
}


/*
    mc68681_channel_a_getc() - read a character from serial channel A, blocking until done.
*/
s16 mc68681_channel_a_getc(dev_t *dev)
{
    mc68681_state_t * const state = (mc68681_state_t *) dev->parent->data;

    /* FIXME: return EAGAIN if FIFO is empty? */
    while(CIRCBUF_IS_EMPTY(state->rxa_buf))
        ;

    return CIRCBUF_READ(state->rxa_buf);
}


/*
    mc68681_channel_b_getc() - read a character from serial channel B, blocking until done.
*/
s16 mc68681_channel_b_getc(dev_t *dev)
{
    mc68681_state_t * const state = (mc68681_state_t *) dev->parent->data;

    /* FIXME: return EAGAIN if FIFO is empty? */
    while(CIRCBUF_IS_EMPTY(state->rxb_buf))
        ;

    return CIRCBUF_READ(state->rxb_buf);
}


/*
    mc68681_read_ip() - read the levels of the input pins IP6-0.
*/
u8 mc68681_read_ip(dev_t *dev)
{
    return MC68681_REG(dev->base_addr, MC68681_IP);
}


/*
    mc68681_set_op_bits() - set the specified output pins OP7-0 to logic 1.
    Note: this function only sets bits; it does not reset them.
*/
void mc68681_set_op_bits(dev_t *dev, ku8 bits)
{
    MC68681_REG(dev->base_addr, MC68681_ROPR) = bits;
}


/*
    mc68681_reset_op_bits() - set the specified output pins OP7-0 to logic 0.
    Note: this function only resets bits; it does not set them.
*/
void mc68681_reset_op_bits(dev_t *dev, ku8 bits)
{
    MC68681_REG(dev->base_addr, MC68681_SOPR) = bits;
}
