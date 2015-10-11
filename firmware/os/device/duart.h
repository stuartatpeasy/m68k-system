#ifndef DUART_H_INC
#define DUART_H_INC
/*
	MC68681 DUART driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "include/defs.h"
#include "include/types.h"

/*
	Base address of MC68681 DUART IC
*/
#define DUART_BASE		((u8 *) 0x00e00000)

/* Yields pointer to DUART register 'r' */
#define DUART_REG(r)    *((vu8 *) (DUART_BASE + ((r) << 1) + 1))

/* DUART crystal clock frequency in Hz */
#define DUART_CLK_HZ    (3686400)


/*
	DUART registers

	offset			read								write
	--------------------------------------------------------------------------------------------
	0				Mode Register A (MR1A, MR2A)		Mode Register A (MR1A, MR2A)			 */
#define DUART_MRA		DUART_REG(0)

/*	1				Status Register A (SRA)				Clock Select Register A (CSRA)			 */
#define DUART_SRA		DUART_REG(1)
#define DUART_CSRA		DUART_REG(1)

/*	2				BRG test							Command register A (CRA))				 */
#define DUART_BRG_TEST	DUART_REG(2)
#define DUART_CRA		DUART_REG(2)

/*	3				Rx Holding Register A (RHRA)		Tx Holding Register A (THRA))			 */
#define DUART_RHRA		DUART_REG(3)
#define DUART_THRA		DUART_REG(3)

/*	4				Input Port Change Register (IPCR)	Aux Control Register (ACR))				 */
#define DUART_IPCR		DUART_REG(4)
#define DUART_ACR		DUART_REG(4)

/*	5				Interrupt Status Register (ISR)		Interrupt Mask Register (IMR))			 */
#define DUART_ISR		DUART_REG(5)
#define DUART_IMR		DUART_REG(5)

/*	6				Counter/Timer Upper Value (CTU)		Counter/Timer Upper Preset Value (CTUR)) */
#define DUART_CTU		DUART_REG(6)
#define DUART_CTUR		DUART_REG(6)

/*	7				Counter/Timer Lower Value (CTL)		Counter/Timer Lower Preset Value (CTLR)) */
#define DUART_CTL		DUART_REG(7)
#define DUART_CTLR		DUART_REG(7)

/*	8				Mode Register B (MR1B, MR2B)		Mode Register B (MR1B, MR2B))			 */
#define DUART_MRB		DUART_REG(8)

/*	9				Status Register B (SRB)				Clock Select Register B (CSRB))			 */
#define DUART_SRB		DUART_REG(9)
#define DUART_CSRB		DUART_REG(9)

/*	10				1x/16x Test							Command Register B (CRB))				 */
#define DUART_1_16_TEST	DUART_REG(10)
#define DUART_CRB		DUART_REG(10)

/*	11				Rx Holding Register B (RHRB)		Tx Holding Register B (THRB))			 */
#define DUART_RHRB		DUART_REG(11)
#define DUART_THRB		DUART_REG(11)

/*	12				Interrupt Vector Register (IVR)		Interrupt Vector Register (IVR))		 */
#define DUART_IVR		DUART_REG(12)

/*	13				Input Ports IP0-IP6					Output Port Conf Register (OPCR))		 */
#define DUART_IP		DUART_REG(13)
#define DUART_OPCR		DUART_REG(13)

/*	14				Start Counter Command (START_CC)	Set Output Port Bits Command (SOPR))	 */
#define DUART_START_CC	DUART_REG(14)
#define DUART_SOPR		DUART_REG(14)

/*	15				Stop Counter Command (STOP_CC)		Reset Output Port Bits Command (ROPR))	 */
#define DUART_STOP_CC	DUART_REG(15)
#define DUART_ROPR		DUART_REG(15)


/*
	DUART register bits
*/

/* DUART_SR[AB] */
#define DUART_SR_RECEIVED_BREAK		(7)
#define DUART_SR_FRAMING_ERROR		(6)
#define DUART_SR_PARITY_ERROR		(5)
#define DUART_SR_OVERRUN_ERROR		(4)
#define DUART_SR_TXEMT				(3)
#define DUART_SR_TXRDY				(2)
#define DUART_SR_FFULL				(1)
#define DUART_SR_RXRDY				(0)

#endif
