#ifndef MC68681_H_INC
#define MC68681_H_INC
/*
	MC68681 MC68681 driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include <include/defs.h>
#include <include/error.h>
#include <include/types.h>

/*
	Base address of MC68681 MC68681 IC
*/
#define MC68681_BASE	    ((u8 *) 0x00e00000)

/* Yields pointer to MC68681 register 'r' */
#define MC68681_REG(r)      *((vu8 *) (MC68681_BASE + ((r) << 1) + 1))

/* MC68681 crystal clock frequency in Hz */
#define MC68681_CLK_HZ      (3686400)


/*
	MC68681 registers

	offset			read								write
	--------------------------------------------------------------------------------------------
	0				Mode Register A (MR1A, MR2A)		Mode Register A (MR1A, MR2A)			 */
#define MC68681_MRA		    MC68681_REG(0)

/*	1				Status Register A (SRA)				Clock Select Register A (CSRA)			 */
#define MC68681_SRA		    MC68681_REG(1)
#define MC68681_CSRA		MC68681_REG(1)

/*	2				BRG test							Command register A (CRA))				 */
#define MC68681_BRG_TEST	MC68681_REG(2)
#define MC68681_CRA		    MC68681_REG(2)

/*	3				Rx Holding Register A (RHRA)		Tx Holding Register A (THRA))			 */
#define MC68681_RHRA		MC68681_REG(3)
#define MC68681_THRA		MC68681_REG(3)

/*	4				Input Port Change Register (IPCR)	Aux Control Register (ACR))				 */
#define MC68681_IPCR		MC68681_REG(4)
#define MC68681_ACR		    MC68681_REG(4)

/*	5				Interrupt Status Register (ISR)		Interrupt Mask Register (IMR))			 */
#define MC68681_ISR		    MC68681_REG(5)
#define MC68681_IMR		    MC68681_REG(5)

/*	6				Counter/Timer Upper Value (CTU)		Counter/Timer Upper Preset Value (CTUR)) */
#define MC68681_CTU		    MC68681_REG(6)
#define MC68681_CTUR		MC68681_REG(6)

/*	7				Counter/Timer Lower Value (CTL)		Counter/Timer Lower Preset Value (CTLR)) */
#define MC68681_CTL		    MC68681_REG(7)
#define MC68681_CTLR		MC68681_REG(7)

/*	8				Mode Register B (MR1B, MR2B)		Mode Register B (MR1B, MR2B))			 */
#define MC68681_MRB		    MC68681_REG(8)

/*	9				Status Register B (SRB)				Clock Select Register B (CSRB))			 */
#define MC68681_SRB		    MC68681_REG(9)
#define MC68681_CSRB		MC68681_REG(9)

/*	10				1x/16x Test							Command Register B (CRB))				 */
#define MC68681_1_16_TEST	MC68681_REG(10)
#define MC68681_CRB		    MC68681_REG(10)

/*	11				Rx Holding Register B (RHRB)		Tx Holding Register B (THRB))			 */
#define MC68681_RHRB		MC68681_REG(11)
#define MC68681_THRB		MC68681_REG(11)

/*	12				Interrupt Vector Register (IVR)		Interrupt Vector Register (IVR))		 */
#define MC68681_IVR		    MC68681_REG(12)

/*	13				Input Ports IP0-IP6					Output Port Conf Register (OPCR))		 */
#define MC68681_IP		    MC68681_REG(13)
#define MC68681_OPCR		MC68681_REG(13)

/*	14				Start Counter Command (START_CC)	Set Output Port Bits Command (SOPR))	 */
#define MC68681_START_CC	MC68681_REG(14)
#define MC68681_SOPR		MC68681_REG(14)

/*	15				Stop Counter Command (STOP_CC)		Reset Output Port Bits Command (ROPR))	 */
#define MC68681_STOP_CC	    MC68681_REG(15)
#define MC68681_ROPR		MC68681_REG(15)


/*
	MC68681 register bits
*/

/* MC68681_SR[AB] */
#define MC68681_SR_RECEIVED_BREAK		(7)
#define MC68681_SR_FRAMING_ERROR		(6)
#define MC68681_SR_PARITY_ERROR		    (5)
#define MC68681_SR_OVERRUN_ERROR		(4)
#define MC68681_SR_TXEMT				(3)
#define MC68681_SR_TXRDY				(2)
#define MC68681_SR_FFULL				(1)
#define MC68681_SR_RXRDY				(0)


s32 mc68681_init(void);

int mc68681_putc(ku32 channel, const char c);
int mc68681_getc(ku32 channel);

void mc68681_start_counter(void);
void mc68681_stop_counter(void);

#endif
