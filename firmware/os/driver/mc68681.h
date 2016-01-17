#ifndef DRIVER_MC68681_H_INC
#define DRIVER_MC68681_H_INC
/*
	MC68681 MC68681 driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/error.h>
#include <kernel/include/types.h>
#include <kernel/util/circbuf.h>


/*
	Base address of MC68681 DUART
*/

#define MC68681_REG_BASE        (1)         /* Regs are 8-bit, located at odd addresses         */
#define MC68681_REG_SHIFT       (1)         /* Regs are 8-bit, located at odd addresses         */

/* This macro provides the offset of register x from the start of the controller's memory map */
#define MC68681_REG_OFFSET(x)       (MC68681_REG_BASE + (((u32) (x)) << MC68681_REG_SHIFT))

#define MC68681_REG_ADDR(base, x)   (((vu8 *) base) + MC68681_REG_OFFSET(x))

/* This macro is an accessor for register x, given a controller at address "base" */
#define MC68681_REG(base, x)        *((vu8 *) MC68681_REG_ADDR((base), (x)))

/* MC68681 crystal clock frequency in Hz */
#define MC68681_CLK_HZ      (3686400)


/*
	MC68681 registers

	offset			read								write
	--------------------------------------------------------------------------------------------
	0				Mode Register A (MR1A, MR2A)		Mode Register A (MR1A, MR2A)			 */
#define MC68681_MRA		    (0)

/*	1				Status Register A (SRA)				Clock Select Register A (CSRA)			 */
#define MC68681_SRA		    (1)
#define MC68681_CSRA		(1)

/*	2				BRG test							Command register A (CRA))				 */
#define MC68681_BRG_TEST	(2)
#define MC68681_CRA		    (2)

/*	3				Rx Holding Register A (RHRA)		Tx Holding Register A (THRA))			 */
#define MC68681_RHRA		(3)
#define MC68681_THRA		(3)

/*	4				Input Port Change Register (IPCR)	Aux Control Register (ACR))				 */
#define MC68681_IPCR		(4)
#define MC68681_ACR		    (4)

/*	5				Interrupt Status Register (ISR)		Interrupt Mask Register (IMR))			 */
#define MC68681_ISR		    (5)
#define MC68681_IMR		    (5)

/*	6				Counter/Timer Upper Value (CTU)		Counter/Timer Upper Preset Value (CTUR)) */
#define MC68681_CTU		    (6)
#define MC68681_CTUR		(6)

/*	7				Counter/Timer Lower Value (CTL)		Counter/Timer Lower Preset Value (CTLR)) */
#define MC68681_CTL		    (7)
#define MC68681_CTLR		(7)

/*	8				Mode Register B (MR1B, MR2B)		Mode Register B (MR1B, MR2B))			 */
#define MC68681_MRB		    (8)

/*	9				Status Register B (SRB)				Clock Select Register B (CSRB))			 */
#define MC68681_SRB		    (9)
#define MC68681_CSRB		(9)

/*	10				1x/16x Test							Command Register B (CRB))				 */
#define MC68681_1_16_TEST	(10)
#define MC68681_CRB		    (10)

/*	11				Rx Holding Register B (RHRB)		Tx Holding Register B (THRB))			 */
#define MC68681_RHRB		(11)
#define MC68681_THRB		(11)

/*	12				Interrupt Vector Register (IVR)		Interrupt Vector Register (IVR))		 */
#define MC68681_IVR		    (12)

/*	13				Input Ports IP0-IP6					Output Port Conf Register (OPCR))		 */
#define MC68681_IP		    (13)
#define MC68681_OPCR		(13)

/*	14				Start Counter Command (START_CC)	Set Output Port Bits Command (SOPR))	 */
#define MC68681_START_CC	(14)
#define MC68681_SOPR		(14)

/*	15				Stop Counter Command (STOP_CC)		Reset Output Port Bits Command (ROPR))	 */
#define MC68681_STOP_CC	    (15)
#define MC68681_ROPR		(15)


/*
	MC68681 register bits
*/

/* MC68681_ACR - auxiliary control register */
#define MC68681_ACR_BRG_SELECT          (7)     /* Baud rate gen select: 0 = set 1, 1 = set 2   */

#define MC68681_ACR_CT_MODE_MASK        (0x03)  /* Counter/timer mode and source                */
#define MC68681_ACR_CT_MODE_SHIFT       (4)

#define MC68681_ACR_DELTA_IP3_INT       (3)     /* Interrupt on change of IP3                   */
#define MC68681_ACR_DELTA_IP2_INT       (2)     /* Interrupt on change of IP2                   */
#define MC68681_ACR_DELTA_IP1_INT       (1)     /* Interrupt on change of IP1                   */
#define MC68681_ACR_DELTA_IP0_INT       (0)     /* Interrupt on change of IP0                   */

/* MC68681_CR - command register */
#define MC68681_CR_MISC_CMD_MASK        (0x70)  /* Miscellaneous commands                       */
#define MC68681_CR_MISC_CMD_SHIFT       (4)

#define MC68681_CR_TX_CMD_MASK          (0x0c)  /* Transmitter commands                         */
#define MC68681_CR_TX_CMD_SHIFT         (2)

#define MC68681_CR_RX_CMD_MASK          (0x03)  /* Receiver commands                            */
#define MC68681_CR_RX_CMD_SHIFT         (0)

/* MC68681_CSR[AB] - clock select register A/B */
#define MC68681_CSR_RXCLK_MASK          (0xf0)  /* Receiver clock select                        */
#define MC68681_CSR_RXCLK_SHIFT         (4)

#define MC68681_CSR_TXCLK_MASK          (0x0f)  /* Transmitter clock select                     */
#define MC68681_CSR_TXCLK_SHIFT         (0)

/* MC68681_IMR - interrupt mask register */
#define MC68681_IMR_INP_CHANGE          (7)     /* Input port change interrupt                  */
#define MC68681_IMR_BRK_B               (6)     /* Delta break B interrupt                      */
#define MC68681_IMR_RXRDY_FFULL_B       (5)     /* RX ready / FFULL channel B interrupt         */
#define MC68681_IMR_TXRDY_B             (4)     /* TX ready channel B interrupt                 */
#define MC68681_IMR_COUNTER_RDY         (3)     /* Counter ready interrupt                      */
#define MC68681_IMR_BRK_A               (2)     /* Delta break A interrupt                      */
#define MC68681_IMR_RXRDY_FFULL_A       (1)     /* RX ready / FFULL channel A interrupt         */
#define MC68681_IMR_TXRDY_A             (0)     /* TX ready channel A interrupt                 */

/* MC68681_MR1[AB] - mode register 1 */
#define MC68681_MR1_RXRTS               (7)     /* RX RTS enable                                */
#define MC68681_MR1_RXIRQ               (6)     /* Specify RX IRQ type: 0=RXRDY, 1=FFULL        */
#define MC68681_MR1_ERR_MODE            (5)     /* Error mode: 0=char, 1=block                  */

#define MC68681_MR1_PARITY_MODE_MASK    (0x18)  /* Parity mode                                  */
#define MC68681_MR1_PARITY_MODE_SHIFT   (3)

#define MC68681_MR1_PARITY_TYPE         (2)     /* Parity type                                  */

#define MC68681_MR1_BPC_MASK            (0x03)  /* Bits per character                           */
#define MC68681_MR1_BPC_SHIFT           (0)

/* MC68681_MR2[AB] - mode register 2 */
#define MC68681_MR2_CHAN_MODE_MASK      (0xc0)  /* Channel mode                                 */
#define MC68681_MR2_CHAN_MODE_SHIFT     (6)

#define MC68681_MR2_TXRTS               (5)     /* TX RTS enable                                */
#define MC68681_MR2_CTS                 (4)     /* Enable transmitter                           */

#define MC68681_MR2_STOP_BIT_LEN_MASK   (0x0f)  /* Stop bit length                              */
#define MC68681_MR2_STOP_BIT_LEN_SHIFT  (0)

/* MC68681_SR[AB] - status register */
#define MC68681_SR_RECEIVED_BREAK		(7)
#define MC68681_SR_FRAMING_ERROR		(6)
#define MC68681_SR_PARITY_ERROR		    (5)
#define MC68681_SR_OVERRUN_ERROR		(4)
#define MC68681_SR_TXEMT				(3)
#define MC68681_SR_TXRDY				(2)
#define MC68681_SR_FFULL				(1)
#define MC68681_SR_RXRDY				(0)


/* Parity modes (see MC68681_MR1_*) */
#define MC68681_PARITY_MODE_ON          (0)
#define MC68681_PARITY_MODE_FORCE       (1)
#define MC68681_PARITY_MODE_NONE        (2)
#define MC68681_PARITY_MODE_MULTIDROP   (3)

/* Parity types (see MC68681_MR1_*) */
#define MC68681_PARITY_TYPE_EVEN        (0)
#define MC68681_PARITY_TYPE_ODD         (1)

/* Bits per character (see MC68681_MR1_*) */
#define MC68681_BPC_5                   (0)     /* 5 bits per character                         */
#define MC68681_BPC_6                   (1)     /* 6 bits per character                         */
#define MC68681_BPC_7                   (2)     /* 7 bits per character                         */
#define MC68681_BPC_8                   (3)     /* 8 bits per character                         */

/* Channel mode (see MC68681_MR2_*) */
#define MC68681_CHAN_MODE_NORMAL        (0)     /* Normal operation                             */
#define MC68681_CHAN_MODE_AUTO_ECHO     (1)     /* Automatic echo                               */
#define MC68681_CHAN_MODE_LOCAL_LOOP    (2)     /* Local loopback                               */
#define MC68681_CHAN_MODE_REMOTE_LOOP   (3)     /* Remote loopback                              */

/* Stop bit length (see MC68681_MR2_*) */
#define MC68681_STOP_BIT_0_563          (0)     /* Stop bit length = 0.563 bits                 */
#define MC68681_STOP_BIT_0_625          (1)     /* Stop bit length = 0.625 bits                 */
#define MC68681_STOP_BIT_0_688          (2)     /* Stop bit length = 0.688 bits                 */
#define MC68681_STOP_BIT_0_750          (3)     /* Stop bit length = 0.750 bits                 */
#define MC68681_STOP_BIT_0_813          (4)     /* Stop bit length = 0.813 bits                 */
#define MC68681_STOP_BIT_0_875          (5)     /* Stop bit length = 0.875 bits                 */
#define MC68681_STOP_BIT_0_938          (6)     /* Stop bit length = 0.938 bits                 */
#define MC68681_STOP_BIT_1_000          (7)     /* Stop bit length = 1.000 bits                 */
#define MC68681_STOP_BIT_1_563          (8)     /* Stop bit length = 1.563 bits                 */
#define MC68681_STOP_BIT_1_625          (9)     /* Stop bit length = 1.625 bits                 */
#define MC68681_STOP_BIT_1_688          (10)    /* Stop bit length = 1.688 bits                 */
#define MC68681_STOP_BIT_1_750          (11)    /* Stop bit length = 1.750 bits                 */
#define MC68681_STOP_BIT_1_813          (12)    /* Stop bit length = 1.813 bits                 */
#define MC68681_STOP_BIT_1_875          (13)    /* Stop bit length = 1.875 bits                 */
#define MC68681_STOP_BIT_1_938          (14)    /* Stop bit length = 1.938 bits                 */
#define MC68681_STOP_BIT_2_000          (15)    /* Stop bit length = 2.000 bits                 */

/* Miscellaneous commands (see MC68681_CR_*) */
#define MC68681_CMD_NONE                (0)     /* No command                                   */
#define MC68681_CMD_RESET_MR_PTR        (1)     /* Reset MR pointer to MR1                      */
#define MC68681_CMD_RESET_RX            (2)     /* Reset receiver                               */
#define MC68681_CMD_RESET_TX            (3)     /* Reset transmitter                            */
#define MC68681_CMD_RESET_ERR           (4)     /* Reset error status                           */
#define MC68681_CMD_RESET_BC_INT        (5)     /* Reset break-change interrupt                 */
#define MC68681_CMD_START_BREAK         (6)     /* Start break                                  */
#define MC68681_CMD_STOP_BREAK          (7)     /* Stop break                                   */

/* Transmitter commands (see MC68681_CR_*) */
#define MC68681_CMD_TX_NONE             (0)     /* No action; stay in current mode              */
#define MC68681_CMD_TX_ENABLE           (1)     /* Enable transmitter                           */
#define MC68681_CMD_TX_DISABLE          (2)     /* Disable transmitter                          */
/*                                      (3)        Do not use - indeterminate results           */

/* Receiver commands (see MC68681_CR_*) */
#define MC68681_CMD_RX_NONE             (0)     /* No action; stay in current mode              */
#define MC68681_CMD_RX_ENABLE           (1)     /* Enable receiver                              */
#define MC68681_CMD_RX_DISABLE          (2)     /* Disable receiver                             */
/*                                      (3)        Do not use - indeterminate results           */

/* Counter/timer modes and sources (see MC68681_ACR_*) */
#define MC68681_CT_MODE_C_EXT           (0)     /* Counter; clock = IP2                         */
#define MC68681_CT_MODE_C_TXCA          (1)     /* Counter; clock = channel A transmitter       */
#define MC68681_CT_MODE_C_TXCB          (2)     /* Counter; clock = channel B transmitter       */
#define MC68681_CT_MODE_C_XTAL16        (3)     /* Counter; clock = crystal / 16                */
#define MC68681_CT_MODE_T_EXT           (4)     /* Timer; clock = IP2                           */
#define MC68681_CT_MODE_T_EXT16         (5)     /* Timer; clock = IP2 / 16                      */
#define MC68681_CT_MODE_T_XTAL          (6)     /* Timer; clock = crystal                       */
#define MC68681_CT_MODE_T_XTAL16        (7)     /* Timer; clock = crystal / 16                  */


typedef struct
{
    u32     rate;           /* Actual baud rate                                         */
    u8      csr;            /* CSR (clock select register) setting, and ACR[7] value    */
} mc68681_baud_rate_entry;


typedef enum mc68681_pin
{
    mc68681_pin_op0 = 0,
    mc68681_pin_op1 = 1,
    mc68681_pin_op2 = 2,
    mc68681_pin_op3 = 3,
    mc68681_pin_op4 = 4,
    mc68681_pin_op5 = 5,
    mc68681_pin_op6 = 6,
    mc68681_pin_op7 = 7
} mc68681_output_pin_t;


typedef enum mc68681_pin_fn
{
    mc68681_pin_fn_gpio,
    mc68681_pin_fn_txa_int,
    mc68681_pin_fn_txb_int,
    mc68681_pin_fn_txrdya_int,
    mc68681_pin_fn_txrdyb_int,
    mc68681_pin_fn_ct_output,
    mc68681_pin_fn_txb_clk,
    mc68681_pin_fn_rxb_clk,
    mc68681_pin_fn_txa16_clk,
    mc68681_pin_fn_txa_clk,
    mc68681_pin_fn_rxa_clk
} mc68681_pin_fn_t;


/* Bits in mc68681_baud_rate_entry.csr */
#define MC68681_BRE_ACR7                BIT(4)  /* Alternate rate gen needed (ACR[7] = 1)   */
#define MC68681_BRE_TEST                BIT(5)  /* Entry requires "BRG test" mode           */
#define MC68681_BRE_CSR_MASK            (0x0f)  /* CSR part only                            */

#define MC68681_CHANNEL_A               (0)
#define MC68681_CHANNEL_B               (1)

#define MC68681_RX_BUF_LEN              (64)
#define MC68681_TX_BUF_LEN              (64)


typedef struct
{
    u8          acr;        /* ACR is write-only, so we store its current val here              */
    u8          opcr;       /* ditto OPCR                                                       */
    u8          brg_test;   /* BRG test mode toggles with each read of the corresponding reg    */
    u8          imr;        /* Interrupt mask register value                                    */
    u32         baud_a;     /* Channel A baud rate                                              */
    u32         baud_b;     /* Channel B baud rate                                              */
    volatile CIRCBUF(u8) rxa_buf;
    volatile CIRCBUF(u8) txa_buf;
    volatile CIRCBUF(u8) rxb_buf;
    volatile CIRCBUF(u8) txb_buf;
} mc68681_state_t;

const mc68681_baud_rate_entry g_mc68681_baud_rates[22];

s32 mc68681_init(dev_t *dev);
s32 mc68681_serial_a_init(dev_t *dev);
s32 mc68681_serial_b_init(dev_t *dev);
s32 mc68681_shut_down(dev_t *dev);
s32 mc68681_set_output_pin_fn(dev_t *dev, const mc68681_output_pin_t pin, const mc68681_pin_fn_t fn);

void mc68681_reset(void * const base_addr);
s32 mc68681_reset_tx(void * const base_addr, ku16 channel);
s32 mc68681_reset_rx(void * const base_addr, ku16 channel);

s32 mc68681_set_baud_rate(dev_t * dev, ku16 channel, ku32 rate);
u32 mc68681_get_baud_rate(dev_t * dev, ku16 channel);
s32 mc68681_control(dev_t *dev, ku32 channel, const devctl_fn_t fn, const void *in, void *out);

s32 mc68681_channel_a_getc(dev_t *dev, char *c);
s32 mc68681_channel_a_putc(dev_t *dev, const char c);
s32 mc68681_channel_a_set_baud_rate(dev_t *dev, ku32 rate);
u32 mc68681_channel_a_get_baud_rate(dev_t *dev);
s32 mc68681_channel_a_control(dev_t *dev, ku32 function, const void *in, void *out);

s32 mc68681_channel_b_getc(dev_t *dev, char *c);
s32 mc68681_channel_b_putc(dev_t *dev, const char c);
s32 mc68681_channel_b_set_baud_rate(dev_t *dev, ku32 rate);
u32 mc68681_channel_b_get_baud_rate(dev_t *dev);

u8 mc68681_read_ip(dev_t *dev);
void mc68681_set_op_bits(dev_t *dev, ku8 bits);
void mc68681_reset_op_bits(dev_t *dev, ku8 bits);


/*
    mc68681_start_counter() - start the MC68681 counter/timer.

    Counter stop/starts may be done as part of context-switching, which needs to be fast, so this
    function is inlined.
*/
inline void mc68681_start_counter(dev_t *dev, ku16 init_count)
{
    void * const base_addr = dev->base_addr;
    u8 dummy;

    /* Set CTUR/CTLR - the counter/timer upper/lower timeout counts */
    MC68681_REG(base_addr, MC68681_CTUR) = (init_count >> 8) & 0xff;
    MC68681_REG(base_addr, MC68681_CTLR) = init_count & 0xff;

    dummy = MC68681_REG(base_addr, MC68681_START_CC);
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}


/*
    mc68681_stop_counter() - stop the MC68681 counter/timer.
    Check the MC68681 data sheet - there may be some oddness related to this.

    Counter stop/starts may be done as part of context-switching, which needs to be fast, so this
    function is inlined.
*/
inline void mc68681_stop_counter(dev_t *dev)
{
    u8 dummy = MC68681_REG(dev->base_addr, MC68681_STOP_CC);
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}


#endif
