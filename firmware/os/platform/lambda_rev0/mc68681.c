/*
	MC68681 DUART "driver"

	(c) Stuart Wallace, December 2011.
*/

#include <platform/lambda_rev0/mc68681.h>


s32 mc68681_init(void)
{
    u8 dummy;

	MC68681_CRA = 0x10;		/* Reset MRA pointer; disable transmitter and receiver	*/
	MC68681_IMR = 0x00;		/* Disable all interrupts								*/

	/*
		Set MR1A:
			bit		val		desc
		----------------------------------------------------------------------
			7		1		RxRTS control
			6		1		RxINT select:  0 = int on RxRDY; 1 = int on FFULL
			5		0		Error mode:  0 = char; 1 = block [?]
			4		1		} Parity mode
			3		0		} 10 = no parity
			2		0		Parity type:  0 = even; 1 = odd.
			1		1		} Bits per character
			0		1		} 11 = 8 bits per character.
	*/
	MC68681_MRA = 0xd3;

	/*
		Set MR2A:
			bit		val		desc
		----------------------------------------------------------------------
			7		0		} Channel mode
			6		0		} 00 = normal
			5		0		TxRTS control
			4		1		CTS enable Tx
			3		0		} Stop bit length
			2		1		}
			1		1		} 0111 = 1.000 bits
			0		1		}
	*/
	MC68681_MRA = 0x17;

	/*
		Set CSRA:
			bits 7-4: receiver clock select; 1100 = 38.4kbaud (with ACR[7] == 0)
			bits 3-0: transmitter clock select; 1100 = 38.4kbaud (with ACR[7] == 0)
	*/
	MC68681_CSRA = 0xcc;

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

	MC68681_ACR = 0x30;

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
	MC68681_CRA = 0x05;


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
	MC68681_OPCR = 0x04;

	/*
		Set OPR - output port bits
		Each bit in the OPR must be set to the complement of the required output pin level.
	*/
	MC68681_SOPR = 0xff;
	MC68681_ROPR = 0x00;

	/* Switch to 115200 baud */
    dummy = MC68681_BRG_TEST;         /* Read BRG test reg to enable "test" baud rates */
    MC68681_CSRA = 0x66;              /* Select 115200 baud */
    dummy += 0;                     /* (silence "set but not used" compiler warning) */

    return SUCCESS;
}


s32 mc68681_putc(ku32 channel, const char c)
{
    if(channel == 0)
    {
        while(!(MC68681_SRA & (1 << MC68681_SR_TXEMT))) ;
        MC68681_THRA = c;
        return c;
    }
    else if(channel == 1)
    {
        while(!(MC68681_SRB & (1 << MC68681_SR_TXEMT))) ;
        MC68681_THRB = c;
        return c;
    }
    else return -EINVAL;
}


int mc68681_getc(ku32 channel)
{
    if(channel == 0)
    {
        while(!(MC68681_SRA & (1 << MC68681_SR_RXRDY))) ;
        return MC68681_RHRA;
    }
    else if(channel == 1)
    {
        while(!(MC68681_SRB & (1 << MC68681_SR_RXRDY))) ;
        return MC68681_RHRB;
    }
    else return -EINVAL;
}


/*
    duart_start_counter() - start the DUART counter
*/
void mc68681_start_counter(void)
{
    u8 dummy;

    /* Set CTUR/CTLR - the counter/timer upper/lower timeout counts */
    MC68681_CTUR = (((MC68681_CLK_HZ / 16) / TICK_RATE) & 0xff00) >> 8;
    MC68681_CTLR = ((MC68681_CLK_HZ / 16) / TICK_RATE) & 0xff;

    dummy = MC68681_START_CC;
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}


/*
    duart_stop_counter() - stop the DUART counter
    Check the MC68681 data sheet - there may be some oddness related to this.
*/
void mc68681_stop_counter(void)
{
    u8 dummy = MC68681_STOP_CC;
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}
