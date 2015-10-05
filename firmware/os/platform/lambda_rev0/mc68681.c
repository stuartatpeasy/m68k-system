/*
	MC68681 DUART "driver"

	(c) Stuart Wallace, December 2011.
*/

#include <platform/lambda_rev0/mc68681.h>


void mc68681_reset(dev_t *dev)
{
    /* Send some initialisation commands to the MC68681 */
    MC68681_REG(dev->base_addr, MC68681_CRA) =  /* 0x10 */
        (MC68681_CMD_RESET_MR_PTR << MC68681_CR_MISC_CMD_SHIFT) |   /* Reset MRA pointer        */
        (MC68681_CMD_TX_DISABLE << MC68681_CR_TX_CMD_SHIFT) |       /* Disable transmitter      */
        (MC68681_CMD_RX_DISABLE << MC68681_CR_RX_CMD_SHIFT);        /* Disable receiver         */

    MC68681_REG(dev->base_addr, MC68681_CRA) = /* 0x20 */
        (MC68681_CMD_RESET_RX << MC68681_CR_MISC_CMD_SHIFT);        /* Reset receiver           */

    MC68681_REG(dev->base_addr, MC68681_CRA) = /* 0x30 */
        (MC68681_CMD_RESET_TX << MC68681_CR_MISC_CMD_SHIFT);        /* Reset transmitter        */
}


s32 mc68681_init(dev_t *dev)
{
    u8 dummy;

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

	/*
		Set CSRA:
			bits 7-4: receiver clock select; 1100 = 38.4kbaud (with ACR[7] == 0)
			bits 3-0: transmitter clock select; 1100 = 38.4kbaud (with ACR[7] == 0)
	*/
	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_CSRA) = 0xcc;

	/*
        Set ACR - auxiliary control register
            bit     val     desc
        ------------------------------------------------------------------------
			7		0		Bit-rate generator set select.  1 = set 2.
            6       0       Counter/timer mode: 0 = counter mode
            5       1       } ACR[5:4] counter/timer clock source
            4       1       } 11 = crystal clock divided by 16
            3       0       IP3 change visible in ISR (0 = not visible)
            2       0       IP2 change visible in ISR (0 = not visible)
            1       0       IP1 change visible in ISR (0 = not visible)
            0       0       IP0 change visible in ISR (0 = not visible)
	*/

	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_ACR) = 0x30;

	/*
		Set CRA:
			bit		val		desc
		----------------------------------------------------------------------
			7		0		Not used; must be 0
			6		0		} Miscellaneous commands
			5		0		}
			4		0		}
			3		0		Disable Tx
			2		1		Enable Tx
			1		0		Disable Rx
			0		1		Enable Rx
	*/
	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_CRA) = 0x05;


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
	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_OPCR) = 0x04;

	/*
		Set OPR - output port bits
		Each bit in the OPR must be set to the complement of the required output pin level.
	*/
	////////////////// TODO ////////////////////
	MC68681_REG(dev->base_addr, MC68681_SOPR) = 0xff;
	MC68681_REG(dev->base_addr, MC68681_ROPR) = 0x00;

	/* Switch to 115200 baud */
    dummy = MC68681_REG(dev->base_addr, MC68681_BRG_TEST);      /* Enable "test" baud rates */
    MC68681_REG(dev->base_addr, MC68681_CSRA) = 0x66;           /* Select 115200 baud       */
    dummy += 0;                     /* (silence "set but not used" compiler warning)        */

    return SUCCESS;
}


s32 mc68681_putc(dev_t *dev, ku32 channel, const char c)
{
    if(channel == 0)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_TXEMT))) ;
        MC68681_REG(dev->base_addr, MC68681_THRA) = c;
        return c;
    }
    else if(channel == 1)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_TXEMT))) ;
        MC68681_REG(dev->base_addr, MC68681_THRB) = c;
        return c;
    }
    else return -EINVAL;
}


int mc68681_getc(dev_t *dev, ku32 channel)
{
    if(channel == 0)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRA) & (1 << MC68681_SR_RXRDY))) ;
        return MC68681_REG(dev->base_addr, MC68681_RHRA);
    }
    else if(channel == 1)
    {
        while(!(MC68681_REG(dev->base_addr, MC68681_SRB) & (1 << MC68681_SR_RXRDY))) ;
        return MC68681_REG(dev->base_addr, MC68681_RHRB);
    }
    else return -EINVAL;
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
