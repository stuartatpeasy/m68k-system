/*
	MC68681 DUART "driver"

	(c) Stuart Wallace, December 2011.
*/

#include <device/mc68681.h>
#include <memory/kmalloc.h>

static void mc68681_set_brg(dev_t *dev, ku8 brg_set, ku8 brg_test);
static void mc68681_set_ct_mode(dev_t *dev, ku8 mode);


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
    mc68681_set_brg() - select a baud-rate generator.  brg_set specifies which of baud-rate sets 0
    and 1 should be used; a nonzero brg_test arg specifies that the "test" baud rates should be
    used.
*/
static void mc68681_set_brg(dev_t *dev, ku8 brg_set, ku8 brg_test)
{
    /* TODO - mutex */
    mc68681_state_t *s = (mc68681_state_t *) dev->data;

    if(brg_set)
        s->acr |= BIT(MC68681_ACR_BRG_SELECT);
    else
        s->acr &= ~BIT(MC68681_ACR_BRG_SELECT);

    MC68681_REG(dev->base_addr, MC68681_ACR) = s->acr;

    if((brg_test && !s->brg_test) || (!brg_test && s->brg_test))
    {
        u8 dummy = MC68681_REG(dev->base_addr, MC68681_BRG_TEST);
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
            }
            else                                /* Channel B */
            {
                MC68681_REG(dev->base_addr, MC68681_CSRB) =
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_RXCLK_SHIFT) |
                    ((p->csr & MC68681_BRE_CSR_MASK) << MC68681_CSR_TXCLK_SHIFT);
            }

            return SUCCESS;
        }
    }

    return EINVAL;      /* No such baud rate */
}


/*
    mc68681_reset_rx() - reset MC68681 receiver
*/
s32 mc68681_reset_rx(dev_t *dev, ku16 channel)
{
    if(channel > MC68681_CHANNEL_B)
        return EINVAL;

    if(channel == MC68681_CHANNEL_A)        /* Channel A */
        MC68681_REG(dev->base_addr, MC68681_CRA) = /* 0x20 */
            (MC68681_CMD_RESET_RX << MC68681_CR_MISC_CMD_SHIFT);
    else                                    /* Channel B */
        MC68681_REG(dev->base_addr, MC68681_CRB) = /* 0x20 */
            (MC68681_CMD_RESET_RX << MC68681_CR_MISC_CMD_SHIFT);

    return SUCCESS;
}


/*
    mc68681_reset_tx() - reset MC68681 transmitter
*/
s32 mc68681_reset_tx(dev_t *dev, ku16 channel)
{
    if(channel > MC68681_CHANNEL_B)
        return EINVAL;

    if(channel == MC68681_CHANNEL_A)        /* Channel A */
        MC68681_REG(dev->base_addr, MC68681_CRA) = /* 0x30 */
            (MC68681_CMD_RESET_TX << MC68681_CR_MISC_CMD_SHIFT);
    else
        MC68681_REG(dev->base_addr, MC68681_CRB) = /* 0x30 */
            (MC68681_CMD_RESET_TX << MC68681_CR_MISC_CMD_SHIFT);

    return SUCCESS;
}


/*
    mc68681_reset() - perform a device reset on the MC68681.  Note: does not reset RX or TX
*/
void mc68681_reset(dev_t *dev)
{
    /* Send some initialisation commands to the MC68681 */
    MC68681_REG(dev->base_addr, MC68681_CRA) =  /* 0x1a */
        (MC68681_CMD_RESET_MR_PTR << MC68681_CR_MISC_CMD_SHIFT) |   /* Reset MRA pointer        */
        (MC68681_CMD_TX_DISABLE << MC68681_CR_TX_CMD_SHIFT) |       /* Disable channel A TX     */
        (MC68681_CMD_RX_DISABLE << MC68681_CR_RX_CMD_SHIFT);        /* Disable channel A RX     */

    MC68681_REG(dev->base_addr, MC68681_CRB) =  /* 0x1a */
        (MC68681_CMD_RESET_MR_PTR << MC68681_CR_MISC_CMD_SHIFT) |   /* Reset MRA pointer        */
        (MC68681_CMD_TX_DISABLE << MC68681_CR_TX_CMD_SHIFT) |       /* Disable channel B TX     */
        (MC68681_CMD_RX_DISABLE << MC68681_CR_RX_CMD_SHIFT);        /* Disable channel B RX     */

    mc68681_reset_tx(dev, MC68681_CHANNEL_A);
    mc68681_reset_tx(dev, MC68681_CHANNEL_B);
    mc68681_reset_rx(dev, MC68681_CHANNEL_A);
    mc68681_reset_rx(dev, MC68681_CHANNEL_B);
}


/*
    mc68681_init() - initialise the MC68681 DUART
*/
s32 mc68681_init(dev_t *dev)
{
    dev->data = CHECKED_KCALLOC(1, sizeof(mc68681_state_t));

    mc68681_reset(dev);

    MC68681_REG(dev->base_addr, MC68681_IMR) = 0x00;    /* Disable all interrupts               */

	/* Set mode register 1A */
	MC68681_REG(dev->base_addr, MC68681_MRA) = /* 0xd3 */
        BIT(MC68681_MR1_RXRTS) |                                        /* Enable RX RTS        */
        BIT(MC68681_MR1_RXIRQ) |                                        /* RXINT: IRQ on FFULL  */
        (MC68681_PARITY_MODE_NONE << MC68681_MR1_PARITY_MODE_SHIFT) |   /* No parity            */
        (MC68681_BPC_8 << MC68681_MR1_BPC_SHIFT);                       /* 8 bits per character */

	/*
        Set mode register 2A

        Note: the MC68681 uses  the same address for MR1A and MR2A.  Having written to MR1A (above),
        an internal pointer in the IC switches such that the next access will address MR2A.
    */
	MC68681_REG(dev->base_addr, MC68681_MRA) = /* 0x17 */
        (MC68681_CHAN_MODE_NORMAL << MC68681_MR2_CHAN_MODE_SHIFT) |
        BIT(MC68681_MR2_CTS) |
        (MC68681_STOP_BIT_1_000 << MC68681_MR2_STOP_BIT_LEN_SHIFT);

    /* Set baud rate generator clock source to the external crystal clock divided by 16 */
    mc68681_set_ct_mode(dev, MC68681_CT_MODE_C_XTAL16);             /* BRG source = xtal/16 */

	/* Enable the channel A transmitter and receiver */
	MC68681_REG(dev->base_addr, MC68681_CRA) = /* 0x05 */
        (MC68681_CMD_TX_ENABLE << MC68681_CR_TX_CMD_SHIFT) |
        (MC68681_CMD_RX_ENABLE << MC68681_CR_RX_CMD_SHIFT);

	/* Enable the channel B transmitter and receiver */
	MC68681_REG(dev->base_addr, MC68681_CRB) = /* 0x05 */
        (MC68681_CMD_TX_ENABLE << MC68681_CR_TX_CMD_SHIFT) |
        (MC68681_CMD_RX_ENABLE << MC68681_CR_RX_CMD_SHIFT);

	/*
		Set OPCR - output port function select
			bit		val		desc
		----------------------------------------------------------------------
			7		0		OP7 - 0: complement of OPR7; 1: TxRDYB interrupt
			6		0		OP6 - 0: complement of OPR6; 1: TxRDYA interrupt
			5		0		OP5 - 0: complement of OPR5; 1: TxB interrupt
			4		0		OP4 - 0: complement of OPR4; 1: TxA interrupt
			3		0		} OP3 - 00: complement of OPR3; 01: C/T output
			2		1		}       10: ch B Tx clk; 11: ch B Rx clk
			1		0		} OP2 - 00: complement of OPR2; 01: ch A Tx 16x clk
			0		0		}       10: ch A Tx 1x clk; 11: ch A Rx 1x clk
	*/
    /////////////// FIXME - platform-specific code (sets OP3 to timer interrupt output) //////////
	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_OPCR) = 0x04;

	/*
		Set OPR - output port bits
		Each bit in the OPR must be set to the complement of the required output pin level.
	*/
	MC68681_REG(dev->base_addr, MC68681_SOPR) = 0xff;
	MC68681_REG(dev->base_addr, MC68681_ROPR) = 0x00;

	/* Set channel A baud rate to 38400 */
	s32 ret = mc68681_set_baud_rate(dev, MC68681_CHANNEL_A, 115200);

    return ret;
}


/*
    mc68681_channel_a_putc() - write a character to serial channel A, blocking until done.
*/
s16 mc68681_channel_a_putc(dev_t *dev, const char c)
{
    while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_TXEMT)))
		;

    MC68681_REG(dev->base_addr, MC68681_THRA) = c;

    return c;
}


/*
    mc68681_channel_b_putc() - write a character to serial channel B, blocking until done.
*/
s16 mc68681_channel_b_putc(dev_t *dev, const char c)
{
    while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_TXEMT)))
		;

	MC68681_REG(dev->base_addr, MC68681_THRB) = c;

    return c;
}


/*
    mc68681_putc() - write a character to the specified serial channel, blocking until done.
*/
s16 mc68681_putc(dev_t *dev, ku16 channel, const char c)
{
    if(channel == 0)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_TXEMT)))
			;

        MC68681_REG(dev->base_addr, MC68681_THRA) = c;
    }
    else if(channel == 1)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_TXEMT)))
			;

		MC68681_REG(dev->base_addr, MC68681_THRB) = c;
    }
    else
		return -EINVAL;

	return c;
}


/*
    mc68681_channel_a_getc() - read a character from serial channel A, blocking until done.
*/
s16 mc68681_channel_a_getc(dev_t *dev)
{
    while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_RXRDY)))
		;

    return MC68681_REG(dev->base_addr, MC68681_RHRA);
}


/*
    mc68681_channel_b_getc() - read a character from serial channel B, blocking until done.
*/
s16 mc68681_channel_b_getc(dev_t *dev)
{
    while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_RXRDY)))
		;

    return MC68681_REG(dev->base_addr, MC68681_RHRB);
}


/*
    mc68681_getc() - read a character from the specified serial channel, blocking until done.
*/
s16 mc68681_getc(dev_t *dev, ku16 channel)
{
    if(channel == 0)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_RXRDY)))
			;

        return MC68681_REG(dev->base_addr, MC68681_RHRA);
    }
    else if(channel == 1)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_RXRDY)))
			;

        return MC68681_REG(dev->base_addr, MC68681_RHRB);
    }
    else
		return -EINVAL;
}


/*
    duart_start_counter() - start the DUART counter
*/
void mc68681_start_counter(dev_t *dev)
{
    u8 dummy;

    /* Set CTUR/CTLR - the counter/timer upper/lower timeout counts */
    MC68681_REG(dev->base_addr, MC68681_CTUR) = (((MC68681_CLK_HZ / 16) / TICK_RATE) & 0xff00) >> 8;
    MC68681_REG(dev->base_addr, MC68681_CTLR) = ((MC68681_CLK_HZ / 16) / TICK_RATE) & 0xff;

    dummy = MC68681_REG(dev->base_addr, MC68681_START_CC);
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}


/*
    duart_stop_counter() - stop the DUART counter
    Check the MC68681 data sheet - there may be some oddness related to this.
*/
void mc68681_stop_counter(dev_t *dev)
{
    u8 dummy = MC68681_REG(dev->base_addr, MC68681_STOP_CC);
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}

